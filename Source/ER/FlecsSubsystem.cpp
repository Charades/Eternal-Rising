#include "FlecsSubsystem.h"

#include "FlecsZombieBoid.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"


flecs::world* UFlecsSubsystem::GetEcsWorld() const
{
	return ECSWorld;
}

void UFlecsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	OnTickDelegate = FTickerDelegate::CreateUObject(this, &UFlecsSubsystem::Tick);
	OnTickHandle = FTSTicker::GetCoreTicker().AddTicker(OnTickDelegate);
	
	// Sets title for Flecs Explorer
	char name[] = { "Eternal Rising Flecs System" };
	char* argv = name;
	ECSWorld = new flecs::world(1, &argv);
	GetEcsWorld()->import<flecs::timer>();
	
	//Comment out the Flecs monitor if you're not using it due to performance overhead
	//https://www.flecs.dev/explorer/v3/
	GetEcsWorld()->import<flecs::monitor>();
	GetEcsWorld()->set<flecs::Rest>({});
	GetEcsWorld()->set_threads(4);
	
	// Expose values with names to Flecs Explorer
	GetEcsWorld()->component<FlecsZombie>().member<FVector3d>("Current Position");
	GetEcsWorld()->component<FlecsISMIndex>().member<int>("ISM Render Index");	
	
	UE_LOG(LogTemp, Warning, TEXT("Eternal Rising Flecs System has started!"));

	InitializeFlowFieldActor(GetWorld());
	
	Super::Initialize(Collection);
}

void UFlecsSubsystem::InitFlecs(UStaticMesh* InMesh)
{
	DefaultMesh = InMesh;
	
	UE_LOG(LogTemp, Warning, TEXT("Flecs Horde System Initialized!"));
}

//This function spawns a pawn and assigns it an instanced static mesh component.
void UFlecsSubsystem::SpawnZombieHorde_Implementation(FVector SpawnLocation, float Radius, int32 NumEntities)
{
	// Spawn a new pawn for this horde
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("Spawning at location: %s"), *SpawnLocation.ToString()));

	for (int32 i = 0; i < NumEntities; i++)
	{
		float Angle = i * (2 * PI / NumEntities);
		float X = FMath::Cos(Angle) * Radius;
		float Y = FMath::Sin(Angle) * Radius;
		FVector PointLocation = SpawnLocation + FVector(X, Y, 0.0f);

		AFlecsZombieBoid* Test = GetWorld()->SpawnActor<AFlecsZombieBoid>(AFlecsZombieBoid::StaticClass(), PointLocation, FRotator::ZeroRotator, SpawnInfo);
		
		DrawDebugSphere(GetWorld(), PointLocation, 20.0f, 12, FColor::Red, false, 10.0f);
	}
}

void UFlecsSubsystem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UFlecsSubsystem, DefaultMesh);
	DOREPLIFETIME(UFlecsSubsystem, AgentInstances);
	DOREPLIFETIME(UFlecsSubsystem, FlowFieldActor);
}

void UFlecsSubsystem::InitializeFlowFieldActor(UWorld* World)
{
	FlowFieldActor = Cast<AFlowFieldWorld>(UGameplayStatics::GetActorOfClass(World, AFlowFieldWorld::StaticClass()));
}

void UFlecsSubsystem::Deinitialize()
{
	FTSTicker::GetCoreTicker().RemoveTicker(OnTickHandle);
	
	if (ECSWorld)
	{
		delete ECSWorld;
		ECSWorld = nullptr;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Eternal Rising Flecs System has shut down!"));
	Super::Deinitialize();
}

// In FlecsSubsystem.cpp
bool UFlecsSubsystem::EnsureFlowFieldActor()
{
    if (!FlowFieldActor)
    {
        FlowFieldActor = Cast<AFlowFieldWorld>(UGameplayStatics::GetActorOfClass(GetWorld(), AFlowFieldWorld::StaticClass()));
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
            FString::Printf(TEXT("FlecsSubsystem: FlowFieldActor found: %s"), 
            FlowFieldActor ? TEXT("Yes") : TEXT("No")));
    }
    return FlowFieldActor != nullptr;
}

void UFlecsSubsystem::ServerInitiateMovement_Implementation(const FVector& TargetLocation, bool bIsEnemyTarget, const TArray<AFlecsZombieBoid*>& Boids)
{
    MulticastExecuteMovement(TargetLocation, bIsEnemyTarget, Boids);
}

void UFlecsSubsystem::MulticastExecuteMovement_Implementation(const FVector& TargetLocation, bool bIsEnemyTarget, const TArray<AFlecsZombieBoid*>& Boids)
{
    // Debug message to verify execution on all clients
    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
        FString::Printf(TEXT("MulticastExecuteMovement on client/server. Boids: %d"), Boids.Num()));

    if (!EnsureFlowFieldActor())
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("FlowFieldActor not found in multicast!"));
        return;
    }

    // Convert FlecsZombieBoid array to Pawn array
    TArray<APawn*> PawnNeighbors;
    for (AFlecsZombieBoid* Boid : Boids)
    {
        if (Boid)
        {
            PawnNeighbors.Add(Cast<APawn>(Boid));
        }
    }

    TMap<FVector2D, FVector> DirectionMap;
    FVector GoalPosition;
    FlowFieldActor->GenerateFlowField(FlowFieldActor->GridCells, TargetLocation, DirectionMap, GoalPosition);

    // Debug message for flow field generation
    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
        FString::Printf(TEXT("Generated flow field. Goal: %s"), *GoalPosition.ToString()));

    // Update movement for each boid
    for (AFlecsZombieBoid* Boid : Boids)
    {
        if (Boid && Boid->FlowFieldMovement)
        {
            // Pass the converted Pawn array
            Boid->FlowFieldMovement->SetExternalNeighbors(PawnNeighbors);
            Boid->FlowFieldMovement->BeginMovement(DirectionMap, GoalPosition);

            if (bIsEnemyTarget)
            {
                // Find closest survivor to the target location
                ASurvivorPawn* ClosestSurvivor = nullptr;
                float ClosestDistance = MAX_FLT;
                
                TArray<AActor*> FoundSurvivors;
                UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASurvivorPawn::StaticClass(), FoundSurvivors);
                
                for (AActor* Actor : FoundSurvivors)
                {
                    float Distance = FVector::Distance(Actor->GetActorLocation(), TargetLocation);
                    if (Distance < ClosestDistance)
                    {
                        ClosestDistance = Distance;
                        ClosestSurvivor = Cast<ASurvivorPawn>(Actor);
                    }
                }
                
                if (ClosestSurvivor)
                {
                    Boid->FlowFieldMovement->SetTargetEnemy(ClosestSurvivor);
                }
            }
            else
            {
                Boid->FlowFieldMovement->ClearTargetEnemy();
            }
        }
    }
}

bool UFlecsSubsystem::Tick(float DeltaTime)
{
	if(ECSWorld)
	{
		ECSWorld->progress(DeltaTime);
	}
	
	return true;
}