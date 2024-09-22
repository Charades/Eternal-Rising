#include "FlecsSubsystem.h"


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
	// Spawn an actor and add an Instanced Static Mesh component to it.
	// Renders the entities
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ZombieRenderer = Cast<UInstancedStaticMeshComponent>((GetWorld()->SpawnActor(AActor::StaticClass(), &FVector::ZeroVector, &FRotator::ZeroRotator, SpawnInfo))->AddComponentByClass(UInstancedStaticMeshComponent::StaticClass(), false, FTransform(FVector::ZeroVector), false));
	ZombieRenderer->SetStaticMesh(InMesh);
	ZombieRenderer->bUseDefaultCollision = false;
	ZombieRenderer->SetGenerateOverlapEvents(false);
	ZombieRenderer->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ZombieRenderer->SetCanEverAffectNavigation(false);
	ZombieRenderer->NumCustomDataFloats = 2;
	
	// Optimized to run every two seconds but could be optimized further by batching
	auto system_adjust_entity_height = GetEcsWorld()->system<FlecsZombie, FlecsISMIndex, FlecsIsmRef>("Zombie Adjust Height")
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

	// auto system_move_to_position = GetEcsWorld()->system<FlecsZombie, FlecsISMIndex, FlecsIsmRef>("Zombie Movement System")
	// 	.iter([](flecs::iter it, FlecsZombie* fw, FlecsISMIndex* fi, FlecsIsmRef* fr) {
	// 		for (int i : it)
	// 		{
	// 		}
	//});
	
	UE_LOG(LogTemp, Warning, TEXT("Flecs Horde System Initialized!"));
}

FFlecsEntityHandle UFlecsSubsystem::SpawnZombieEntity(FVector location, FRotator rotation)
{
	auto IsmID = ZombieRenderer->AddInstance(FTransform(rotation, location));
	auto entity = GetEcsWorld()->entity()
	.set<FlecsIsmRef>({ZombieRenderer})
	.set<FlecsISMIndex>({IsmID})
	.set<FlecsZombie>({FVector(0,0,0)})
	.child_of<Horde>()
	.set_name(StringCast<ANSICHAR>(*FString::Printf(TEXT("Zombie%d"), IsmID)).Get());
	return FFlecsEntityHandle{int(entity.id())};
}

// Experimental
void UFlecsSubsystem::SpawnZombieSquad(UStaticMesh* InMesh, FVector SquadLocation, int32 NumEntities)
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
	AFlecsZombiePawn* SquadPawn = GetWorld()->SpawnActor<AFlecsZombiePawn>(AFlecsZombiePawn::StaticClass(), SquadLocation, FRotator::ZeroRotator, SpawnInfo);
    
	// Set the static mesh for rendering zombies
	SquadPawn->InstancedMeshComponent->SetStaticMesh(InMesh);
    
	// Spawn the entities in Flecs and associate them with the Pawn
	for (int32 i = 0; i < NumEntities; i++)
	{
		FVector EntityLocation = SquadLocation + FVector(i * 100, 0, 0); // Example: Offset each entity by 100 units
		auto IsmID = SquadPawn->InstancedMeshComponent->AddInstance(FTransform(FRotator::ZeroRotator, EntityLocation));
        
		auto entity = GetEcsWorld()->entity()
			.set<FlecsIsmRef>({SquadPawn->InstancedMeshComponent})
			.set<FlecsISMIndex>({IsmID})
			.set<FlecsZombie>({FVector(0,0,0)})
			.child_of<Horde>()
			.set_name(StringCast<ANSICHAR>(*FString::Printf(TEXT("Zombie%d"), IsmID)).Get());

		// Add the entity to the Pawn’s list
		SquadPawn->AddEntityToSquad(entity, IsmID);
	}
	
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