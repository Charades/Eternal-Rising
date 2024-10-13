// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnActor.h"
#include "FlecsSubsystem.h"

// Sets default values
ASpawnActor::ASpawnActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Set this actor to call Tick() every frame if necessary. You can turn this off if not needed.
	PrimaryActorTick.bCanEverTick = false;

	// Create and attach the Spawn Orb
	OrbMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SphereMesh"));
	RootComponent = OrbMesh;
	OrbMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OrbMesh->SetCollisionObjectType(ECC_WorldDynamic);  // Choose an appropriate collision object type
	OrbMesh->SetCollisionResponseToAllChannels(ECR_Ignore);  // Ignore all collision channels by default
	OrbMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);  // Respond to line traces (typically on Visibility channel)
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMeshAsset.Succeeded())
	{
		OrbMesh->SetStaticMesh(SphereMeshAsset.Object);
	}
}

bool ASpawnActor::ShowSpawnMenu(bool bSetToggle)
{
	// Error Handling
	if (!OrbMesh || !MatDefault || !MatSelected) return false;
	
	//SpawnPoints(300.0f, 20);
	
	// Change the material of the sphere when this function is called
	if (bSetToggle && !bMenuToggled)
	{
		OrbMesh->SetMaterial(0, MatSelected);
		bMenuToggled = true;

		// Menu Toggled to True
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("%s Enable Spawn Menu"), *this->GetName()));
		SpawnPoints(1200.0f, 100);
		return true;
	}
    
	// If toggling back to the original material
	if (!bSetToggle && bMenuToggled)
	{
		OrbMesh->SetMaterial(0, MatDefault); 
		bMenuToggled = false;

		// Menu Toggled to False
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("%s Disable Spawn Menu"), *this->GetName()));
		return true; 
	}

	return false;
}

// This is essentially all we'll need for the spawn loc
void ASpawnActor::SpawnPoints(float Radius, int32 NumberOfPoints)
{
	if (NumberOfPoints <= 0) return;

	// Get the location of this actor
	FVector ActorLocation = GetActorLocation();
	//ActorLocation.X = ActorLocation.X + 600.0f;
	ActorLocation.Z = 0;

	UFlecsSubsystem* MySubsystem = GetGameInstance()->GetSubsystem<UFlecsSubsystem>();

	if (MySubsystem)
	{
		MySubsystem->SpawnZombieHorde(ActorLocation, Radius, NumberOfPoints);
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Subsystem not found!"));
		}
	}
}

// Called when the game starts or when spawned
void ASpawnActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASpawnActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

