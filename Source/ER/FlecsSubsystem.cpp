#include "FlecsSubsystem.h"

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
	 auto system_snap_to_surface = GetEcsWorld()->system<FlecsZombie, FlecsISMIndex, FlecsIsmRef>("Zombie Snap To Surface")
	 .interval(2.0)
	 .iter([](flecs::iter it, FlecsZombie* fw, FlecsISMIndex* fi, FlecsIsmRef* fr) {
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
	 			InstanceTransform.SetLocation(InstanceLocation);
	
	 			// Update the instance's transform
	 			fr[i].Value->UpdateInstanceTransform(index, InstanceTransform, true, true, true);
	 		}
	 	}
	 });

	 auto system_movement_behavior = GetEcsWorld()->system<FlecsZombie, FlecsISMIndex, FlecsIsmRef>("Zombie Movement Behavior")
	 	.iter([](flecs::iter it, FlecsZombie* fw, FlecsISMIndex* fi, FlecsIsmRef* fr) {
	 		for (int i : it)
	 		{
	 		}
	});
	
	UE_LOG(LogTemp, Warning, TEXT("Flecs Horde System Initialized!"));
}

// This function spawns a pawn and assigns it an instanced static mesh component.
void UFlecsSubsystem::SpawnZombieHorde(FVector SpawnLocation, float Radius, int32 NumEntities)
{
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
	
	if (!NewHorde)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn AFlecsZombieHorde!"));
		return;
	}

	// Create a new ISM component for this horde
	UHierarchicalInstancedStaticMeshComponent* ZombieRenderer = NewObject<UHierarchicalInstancedStaticMeshComponent>(NewHorde);
	if (!ZombieRenderer)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create UInstancedStaticMeshComponent."));
		NewHorde->Destroy();
		return;
	}

	// Set up the new ISM component
	ZombieRenderer->SetupAttachment(NewHorde->GetRootComponent());
	ZombieRenderer->RegisterComponent();
	ZombieRenderer->SetStaticMesh(DefaultMesh.Get());
	ZombieRenderer->bUseDefaultCollision = false;
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
void UFlecsSubsystem::SpawnZombieEntity(FVector Location, FRotator Rotation, UHierarchicalInstancedStaticMeshComponent* ZombieRendererInst)
{
    // Add the instance of the zombie's mesh to the ISM component
   auto IsmID = ZombieRendererInst->AddInstance(FTransform(Rotation, Location));
	
    // // Create the entity in Flecs
    auto Entity = GetEcsWorld()->entity()
        .set<FlecsIsmRef>({ZombieRendererInst})
        .set<FlecsISMIndex>({IsmID})
        .set<FlecsZombie>({100.0f})
        .child_of<Horde>()  // Parent it to the horde
        .set_name(StringCast<ANSICHAR>(*FString::Printf(TEXT("Zombie%d_%d"), IsmID, ZombieRendererInst->GetOwner()->GetUniqueID())).Get());
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