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
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("%s Enable Spawn Menu"), *this->GetName()));
		SpawnPoints(300.0f, 20);
		return true;
	}
    
	// If toggling back to the original material
	if (!bSetToggle && bMenuToggled)
	{
		OrbMesh->SetMaterial(0, MatDefault); 
		bMenuToggled = false;

		// Menu Toggled to False
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("%s Disable Spawn Menu"), *this->GetName()));
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

	for (int32 i = 0; i < NumberOfPoints; i++)
	{
		// Calculate the angle for the point
		float Angle = i * (2 * PI / NumberOfPoints);

		// Calculate the X and Y coordinates
		float X = FMath::Cos(Angle) * Radius;
		float Y = FMath::Sin(Angle) * Radius;

		// Calculate the final position by adding the offsets to the actor's location
		FVector PointLocation = ActorLocation + FVector(X, Y, 0.0f);

		if (GetGameInstance())
		{
			// Get the subsystem and call the function
			UFlecsSubsystem* MySubsystem = GetGameInstance()->GetSubsystem<UFlecsSubsystem>();

			if (MySubsystem)
			{
				MySubsystem->SpawnZombieEntity(PointLocation, FRotator(0,0,0));
			}
			else
			{
				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Subsystem not found!"));
				}
			}
		}
		
		// Debug for now, replace with zombie entities in the future
		DrawDebugSphere(
			GetWorld(),
			PointLocation,            // Location of the debug sphere
			20.0f,                    // Sphere radius
			12,                       // Number of segments
			FColor::Red,              // Sphere color
			false,                    // Persistent (false means it will disappear after a short time)
			10.0f                     // Lifetime of the sphere
		);
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

