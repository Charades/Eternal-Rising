// Fill out your copyright notice in the Description page of Project Settings.


#include "DirectorPawn.h"

#include "FlowFieldMovement.h"
#include "Engine/OverlapResult.h"


// Sets default values
ADirectorPawn::ADirectorPawn(): SelectionWidget(nullptr), DefaultMappingContext(nullptr), InputData(nullptr)
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ADirectorPawn::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetInputMode(FInputModeGameAndUI());
		PC->bShowMouseCursor = true;

		// Get the Enhanced Input Local Player Subsystem
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			// Add the mapping context
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
			UE_LOG(LogTemp, Log, TEXT("Mapping Context added in BeginPlay"));
		}
	}

	if (MarqueeWidgetClass)
	{
		SelectionWidget = CreateWidget<UMarqueeSelectionWidget>(GetWorld(), MarqueeWidgetClass);
		if (SelectionWidget)
		{
			SelectionWidget->AddToViewport();
		}
	}
    
	FlowFieldActor = Cast<AFlowFieldWorld>(UGameplayStatics::GetActorOfClass(GetWorld(), AFlowFieldWorld::StaticClass()));

	if (!FlowFieldActor)
	{
		UE_LOG(LogTemp, Error, TEXT("FlowFieldActor is null! Ensure there is an instance of AFlowFieldCPP in the level."));
	}
}

// Called every frame
void ADirectorPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (bIsSelecting && SelectionWidget)
	{
		FVector2D CurrentMousePosition;
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			PC->GetMousePosition(CurrentMousePosition.X, CurrentMousePosition.Y);
			SelectionWidget->UpdateMarquee(InitialMousePosition, CurrentMousePosition);
		}
	}
}

// Called to bind functionality to input
void ADirectorPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	// Initialize Enhanced Input
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (InputData)
		{
			EnhancedInputComponent->BindAction(InputData->StartDragEvent, ETriggerEvent::Started, this, &ADirectorPawn::StartMarqueeSelection);
			EnhancedInputComponent->BindAction(InputData->EndDragEvent, ETriggerEvent::Completed, this, &ADirectorPawn::EndMarqueeSelection);
			EnhancedInputComponent->BindAction(InputData->RightMouseClick, ETriggerEvent::Completed, this, &ADirectorPawn::MoveToLocation);
			UE_LOG(LogTemp, Log, TEXT("Input actions bound in SetupPlayerInputComponent"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("InputData or LeftMouseClick action is null in SetupPlayerInputComponent"));
		}
        
		// Add the mapping context
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
				UE_LOG(LogTemp, Log, TEXT("Mapping Context added to Input Subsystem"));
			}
		}
	}
}

void ADirectorPawn::StartMarqueeSelection()
{
	bIsSelecting = true;

	if (SelectionWidget)
	{
		SelectionWidget->SetVisibility(ESlateVisibility::Visible); // Ensure widget is visible
	}

	
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->GetMousePosition(InitialMousePosition.X, InitialMousePosition.Y);
		PC->bShowMouseCursor = true;
		PC->SetInputMode(FInputModeGameAndUI().SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock).SetHideCursorDuringCapture(false));
	}

	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Drag started!"));
}

void ADirectorPawn::EndMarqueeSelection()
{
	bIsSelecting = false;

	if (SelectionWidget)
	{
		SelectionWidget->SetVisibility(ESlateVisibility::Hidden); // Hide instead of destroying
	}
	
	FVector2D CurrentMousePosition;
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->GetMousePosition(CurrentMousePosition.X, CurrentMousePosition.Y);

		PC->SetInputMode(FInputModeGameAndUI());
		PC->bShowMouseCursor = true;
	}
	
	FVector2D TopLeft(FMath::Min(InitialMousePosition.X, CurrentMousePosition.X), FMath::Min(InitialMousePosition.Y, CurrentMousePosition.Y));
	FVector2D BottomRight(FMath::Max(InitialMousePosition.X, CurrentMousePosition.X), FMath::Max(InitialMousePosition.Y, CurrentMousePosition.Y));
	
	PerformSelection(TopLeft, BottomRight);
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Drag ended!"));
}

void ADirectorPawn::MoveToLocation()
{
	FVector WorldLocation, WorldDirection;

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (PC->DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
		{
			FVector Start = WorldLocation;
			FVector End = Start + (WorldDirection * 40000.0f);

			FHitResult HitResult;
			if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility))
			{
				StartPosition = HitResult.ImpactPoint;
				
				if (!PawnMovements.IsEmpty())
				{
					GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("We can move these pawns"));
					TMap<FVector2D, FVector> DirectionMap;
					FVector GoalPosition;
					FlowFieldActor->GenerateFlowField(FlowFieldActor->GridCells, StartPosition, DirectionMap, GoalPosition);

					for (UFlowFieldMovement* Movement : PawnMovements)
					{
						if (Movement)
						{
							Movement->BeginMovement(DirectionMap, GoalPosition);
						}
					}
				}
				else
				{
					FoundStartPosition = true;
					StartPosition = HitResult.ImpactPoint;
				}
				DrawDebugLine(GetWorld(), Start, End, FColor::Blue, false, 1.0f, 0, 1.0f);
			}
			else
			{
				FoundStartPosition = false;
			}
		}
	}
}

void ADirectorPawn::PerformSelection(const FVector2D& TopLeft, const FVector2D& BottomRight)
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC) return;

    // Clear previous selection
    SelectedPawns.Empty();
	PawnMovements.Empty();

    // Get all pawns in the world first
    TArray<AActor*> AllPawns;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), AllPawns);

	// Add a buffer to make selection more forgiving (adjust this value as needed)
	const float SelectionBuffer = 20.0f; // Buffer in pixels

	// Screen bounds for our selection box with buffer
	float MinX = FMath::Min(TopLeft.X, BottomRight.X) - SelectionBuffer;
	float MaxX = FMath::Max(TopLeft.X, BottomRight.X) + SelectionBuffer;
	float MinY = FMath::Min(TopLeft.Y, BottomRight.Y) - SelectionBuffer;
	float MaxY = FMath::Max(TopLeft.Y, BottomRight.Y) + SelectionBuffer;

    // Check each pawn to see if it's in our selection box
    for (AActor* Actor : AllPawns)
    {
        if (APawn* Pawn = Cast<APawn>(Actor))
        {
            if (Pawn == this) continue; // Skip the director pawn itself

            FVector2D ScreenPos;
            if (PC->ProjectWorldLocationToScreen(Pawn->GetActorLocation(), ScreenPos))
            {
                // Check if the pawn's screen position is within our marquee bounds
                if (ScreenPos.X >= MinX && ScreenPos.X <= MaxX &&
                    ScreenPos.Y >= MinY && ScreenPos.Y <= MaxY)
                {
                    // Do an additional visibility check (optional)
                    FHitResult HitResult;
                    FVector PawnLocation = Pawn->GetActorLocation();
                    FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
                    
                    FCollisionQueryParams QueryParams;
                    QueryParams.AddIgnoredActor(this);
                    QueryParams.AddIgnoredActor(Pawn);
                    
                    bool bHasLineOfSight = !World->LineTraceSingleByChannel(
                        HitResult,
                        CameraLocation,
                        PawnLocation,
                        ECC_Visibility,
                        QueryParams
                    );

                    if (bHasLineOfSight)
                    {
                        SelectedPawns.Add(Pawn);

                    	if (UFlowFieldMovement* MovementComponent = Pawn->FindComponentByClass<UFlowFieldMovement>())
                    	{
                    		if (!PawnMovements.Contains(MovementComponent))
                    		{
                    			PawnMovements.Add(MovementComponent);
                    			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red,  FString::Printf(TEXT("Added Movement Component from Pawn: %s"), *Pawn->GetName()));
                    		}
                    		else
                    		{
                    			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red,  FString::Printf(TEXT("Movement Component already exists for Pawn: %s"), *Pawn->GetName()));
                    		}
                    	}
                    	else
                    	{
                    		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("No FlowFieldMovement Component found in Pawn: %s"), *Pawn->GetName()));
                    	}
                    	
                        //UE_LOG(LogTemp, Log, TEXT("Selected Pawn: %s"), *Pawn->GetName());
                    	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red,  *Pawn->GetName());
                    }
                }
            }
        }
    }

    // For debugging: Draw debug lines in world space to show selected pawns
    for (APawn* SelectedPawn : SelectedPawns)
    {
        DrawDebugBox(
            World,
            SelectedPawn->GetActorLocation(),
            FVector(50.0f),
            FQuat::Identity,
            FColor::Green,
            false,
            1.0f
        );
    }
}
