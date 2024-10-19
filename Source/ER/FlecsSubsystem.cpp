#include "FlecsSubsystem.h"

#include "FlecsZombieBoid.h"
#include "Kismet/KismetMathLibrary.h"
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
	Super::Initialize(Collection);
}

void UFlecsSubsystem::InitFlecs(UStaticMesh* InMesh)
{
	DefaultMesh = InMesh;
	
	//Optimized to run every two seconds but could be optimized further by batching
	   auto system_snap_to_surface = GetEcsWorld()->system<FlecsZombie, FlecsISMIndex, FlecsIsmRef, FlecsTargetLocation>("Zombie Snap To Surface")
	   .interval(2.0)
	   .iter([](flecs::iter it, FlecsZombie* fw, FlecsISMIndex* fi, FlecsIsmRef* fr, FlecsTargetLocation* ftl) {
	   	for (int i : it) {
	   		auto index = fi[i].Value;
	   		FTransform InstanceTransform;
	   		
	   		fr[i].Value->GetInstanceTransform(index, InstanceTransform, true);
	  
	   		FVector InstanceLocation = InstanceTransform.GetLocation();
	   		FHitResult HitResult;
	  
	   		// Trace downwards
	   		FVector Start = InstanceLocation;
	   		FVector End = Start - FVector(0.f, 0.f, 1000.f); 
	  
	   		// Ignore the owning actor
	   		FCollisionQueryParams QueryParams;
	   		QueryParams.AddIgnoredActor(fr[i].Value->GetOwner());
	   	
	   		if (fr[i].Value->GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
	   		{
	   			// Adjust the instance position to the hit point
	   			InstanceLocation.Z = HitResult.Location.Z;
	   			ftl[i].Value.Z = HitResult.Location.Z;
	   			InstanceTransform.SetLocation(InstanceLocation);
	  
	   			// Update the instance's transform
	   			bool bMarkRenderState = (i == it.count() - 1);
	   			fr[i].Value->UpdateInstanceTransform(index, InstanceTransform, true, bMarkRenderState, true);
	   		}
	   	}
	   });

	//This is just an example that will be replaced by the boids movement system
	auto system_movement_behavior = GetEcsWorld()->system<FlecsISMIndex, FlecsIsmRef, FlecsTargetLocation, FlecsHordeRef>("Zombie Movement Behavior")
	.iter([](flecs::iter& it, FlecsISMIndex* fi, FlecsIsmRef* fr, FlecsTargetLocation* ftl, FlecsHordeRef* h) {
	    const float DistanceThreshold = 100.0f;
	    const float LerpAlpha = 0.005f;
	    const float RotationLerpAlpha = 0.03f;
	    const FVector2D OffsetRange(-1000.0f, 1000.0f);
	
	    for (int i = 0; i < it.count(); ++i) {
	        const int32 index = fi[i].Value;
	    	const bool bMarkRenderState = (i == it.count() - 1);
	        FTransform InstanceTransform;
	        fr[i].Value->GetInstanceTransform(index, InstanceTransform, true);
	    	
	        FVector PawnLocation = h[i].Value->GetActorLocation();
	        FVector& TargetOffset = ftl[i].Value;
	
	        if (TargetOffset.IsZero()) {
	            TargetOffset = FVector(
	                FMath::RandRange(OffsetRange.X, OffsetRange.Y),
	                FMath::RandRange(OffsetRange.X, OffsetRange.Y),
	                0.0f
	            );
	            InstanceTransform.SetLocation(PawnLocation + TargetOffset);
	            fr[i].Value->UpdateInstanceTransform(index, InstanceTransform, true, bMarkRenderState, true);
	            continue;
	        }
	
	        const FVector InstanceLocation = InstanceTransform.GetLocation();
	        if (FVector::DistSquared(InstanceLocation, PawnLocation + TargetOffset) < DistanceThreshold) {
	            TargetOffset = FVector(
	                FMath::RandRange(OffsetRange.X, OffsetRange.Y),
	                FMath::RandRange(OffsetRange.X, OffsetRange.Y),
	                0.0f
	            );
	        }
	
	        const FVector PawnVelocity = h[i].Value->GetVelocity();
	        if (!PawnVelocity.IsNearlyZero()) {
	            const FVector PawnMovementDirection = PawnVelocity.GetSafeNormal();
	            const FRotator LookAtRotation = UKismetMathLibrary::MakeRotFromX(PawnMovementDirection);
	            const FQuat NewRotation = FQuat::Slerp(
	                InstanceTransform.GetRotation(),
	                FRotator(0.0f, LookAtRotation.Yaw - 90.0f, 0.0f).Quaternion(),
	                RotationLerpAlpha
	            );
	            InstanceTransform.SetRotation(NewRotation);
	        }
	
	        FVector TargetLocation = PawnLocation + TargetOffset;
	        TargetLocation.Z = InstanceLocation.Z;
	        const FVector NewLocation = FMath::Lerp(InstanceLocation, TargetLocation, LerpAlpha);
	        InstanceTransform.SetLocation(NewLocation);
	    	
	        fr[i].Value->UpdateInstanceTransform(index, InstanceTransform, true, bMarkRenderState, true);
	    }
	});
	
	UE_LOG(LogTemp, Warning, TEXT("Flecs Horde System Initialized!"));
}

// This function spawns a pawn and assigns it an instanced static mesh component.
void UFlecsSubsystem::SpawnZombieHorde(FVector SpawnLocation, float Radius, int32 NumEntities)
{
	UE_LOG(LogTemp, Error, TEXT("Accessed SpawnZombieHorde"));
	
	if (!DefaultMesh.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("DefaultMesh is not valid! Make sure it's set correctly."));
		return;
	}

	// Spawn a new pawn for this horde
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("Spawning at location: %s"), *SpawnLocation.ToString()));
	AFlecsZombieHorde* NewHorde = GetWorld()->SpawnActor<AFlecsZombieHorde>(AFlecsZombieHorde::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnInfo);
	CurrentAgentIndex = AgentInstances.AddUnique(NewHorde);
	OnHordeSpawned(NewHorde);
	
	if (!NewHorde)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn AFlecsZombieHorde!"));
		return;
	}

	// Create a new ISM component for this horde
	UInstancedStaticMeshComponent* ZombieRenderer = NewHorde->FindComponentByClass<UInstancedStaticMeshComponent>();
	if (!ZombieRenderer)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create UInstancedStaticMeshComponent."));
		return;
	}

	// Set up the new ISM component
	ZombieRenderer->SetStaticMesh(DefaultMesh.Get());
	ZombieRenderer->SetGenerateOverlapEvents(false);
	ZombieRenderer->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ZombieRenderer->SetCanEverAffectNavigation(false);
	ZombieRenderer->NumCustomDataFloats = 2;

	// Spawn zombie entities
	for (int32 i = 0; i < NumEntities; i++)
	{
		float Angle = i * (2 * PI / NumEntities);
		float X = FMath::Cos(Angle) * Radius;
		float Y = FMath::Sin(Angle) * Radius;
		FVector PointLocation = SpawnLocation + FVector(X, Y, 0.0f);

		SpawnZombieEntity(PointLocation, FRotator::ZeroRotator, ZombieRenderer);

		//DrawDebugSphere(GetWorld(), PointLocation, 20.0f, 12, FColor::Red, false, 10.0f);
	}
}

// This function spawns a single zombie entity and adds it to the instanced static mesh component (ISM)
void UFlecsSubsystem::SpawnZombieEntity(FVector Location, FRotator Rotation, UInstancedStaticMeshComponent* ZombieRendererInst)
{
    // Add the instance of the zombie's mesh to the ISM component
	auto IsmID = ZombieRendererInst->AddInstance(FTransform(Rotation, Location));
	AFlecsZombieHorde* HordeRef = Cast<AFlecsZombieHorde>(ZombieRendererInst->GetOwner());
	
    // // Create the entity in Flecs
    auto Entity = GetEcsWorld()->entity()
		.set<FlecsHordeRef>({HordeRef})
        .set<FlecsIsmRef>({ZombieRendererInst})
        .set<FlecsISMIndex>({IsmID})
        .set<FlecsZombie>({100.0f})
		.set<FlecsTargetLocation>({FVector::ZeroVector})
		.child_of<Horde>()
        .set_name(StringCast<ANSICHAR>(*FString::Printf(TEXT("Zombie%d_%d"), IsmID, ZombieRendererInst->GetOwner()->GetUniqueID())).Get());

	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Added entity"));

	UE_LOG(LogTemp, Error, TEXT("Added Entity"));
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

bool UFlecsSubsystem::Tick(float DeltaTime)
{
	if(ECSWorld)
	{
		ECSWorld->progress(DeltaTime);
	}
	
	return true;
}