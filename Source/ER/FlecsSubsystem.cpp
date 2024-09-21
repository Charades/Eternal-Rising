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
	
	//Comment out the Flecs monitor if you're not using it due to performance overhead
	//https://www.flecs.dev/explorer/
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