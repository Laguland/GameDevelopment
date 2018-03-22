// Fill out your copyright notice in the Description page of Project Settings.

#include "DoorKeySwing.h"
#include "DoorKey.h"
#include "ConstructorHelpers.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"


// Sets default values
ADoorKeySwing::ADoorKeySwing()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* root = CreateDefaultSubobject<USceneComponent>(TEXT("ROOT"));
	RootComponent = root;

	// Create Triger Box
	boxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Comp"));
	boxComp->InitBoxExtent(FVector(150, 100, 100));
	boxComp->SetCollisionProfileName("Trigger");
	boxComp->SetSimulatePhysics(false);
	boxComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	boxComp->SetupAttachment(RootComponent);

	// Add overlap events functions 
	boxComp->OnComponentBeginOverlap.AddDynamic(this, &ADoorKeySwing::OnOverlapBegin);
	boxComp->OnComponentEndOverlap.AddDynamic(this, &ADoorKeySwing::OnOverlapEnd);

	// Add door asset as Box component and set it as root component																		  
	door = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door"));																	  
	door->SetupAttachment(RootComponent);																								  
																																		  
	// Parse asset																														  
	static ConstructorHelpers::FObjectFinder<UStaticMesh> DoorAsset(TEXT("/Game/Assets/Props/BronzeDoor/BronzeDoor.BronzeDoor"));				  
																																		  
	if (DoorAsset.Succeeded())																											  
	{																																	  
		door->SetStaticMesh(DoorAsset.Object);																							  
		door->SetRelativeLocation(FVector(0.0f, 50.0f, -100.0f)); // set relative location to trigger box								  
		door->SetWorldScale3D(FVector(1.0f));																							  
	}																																	  
																																		  
	//// Add door asset as Box component and set it as root component																		  
	//door2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door2"));																	  
	//door2->SetupAttachment(RootComponent);

	//// Parse asset
	//static ConstructorHelpers::FObjectFinder<UStaticMesh> DoorAsset2(TEXT("/Game/StarterContent/Props/SM_Door_2.SM_Door_2"));

	//if (DoorAsset2.Succeeded())
	//{
	//	door2->SetStaticMesh(DoorAsset2.Object);
	//	door2->SetRelativeLocation(FVector(0.0f, 50.0f, -100.0f)); // set relative location to trigger box
	//	door2->SetWorldScale3D(FVector(1.0f));
	//}

	// Set varaibles

	isClosed = true;
	Opening = false;
	Closing = true;
	isKeyInside = false;

	maxDegree = 90.0f;
	doorCurrentRotation = 0.0f;

	// Key Animation Controls

	doorLocation = door->GetComponentToWorld().GetLocation();
	startPosition = FVector().ZeroVector;
	endPosition = FVector().ZeroVector;
	moveVector = FVector().ZeroVector;
	newKeyLocation = FVector().ZeroVector;

	moveToStartingPosition = true;
	moveToEndPosition = false;
	startRotate = false;

	moveSpeed = 60.0f;
	rotationSpeed = 90.0f;

	// Scene Components

	keyStartPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Key Start Point"));
	keyStartPoint->SetupAttachment(door);
	keyStartPoint->SetRelativeLocation(FVector(-40,-80,90));
	keyEndPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Key End Point"));
	keyEndPoint->SetupAttachment(door);
	keyEndPoint->SetRelativeLocation(FVector(0,-80,90));
}

// Called when the game starts or when spawned
void ADoorKeySwing::BeginPlay()
{
	Super::BeginPlay();

	// Draw trigger box for testing
	//DrawDebugBox(GetWorld(), GetActorLocation(), boxComp->GetScaledBoxExtent(), FQuat(GetActorRotation()), FColor::Red, true, -1.0f, 0, 2);
}

// Called every frame
void ADoorKeySwing::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Opening)
	{
		OpenDoor(DeltaTime);
		doorKey->SetActorLocation(keyEndPoint->GetComponentTransform().GetLocation());
	}

	if (Closing)
	{
		CloseDoor(DeltaTime);
	}

	bKeyMovementFinished = KeyMovement(DeltaTime);

	if (bKeyMovementFinished)
	{
		if (isClosed)
		{
			isClosed = false;
			Closing = false;
			Opening = true;
		}
	}
}

// Opend Door Function

void ADoorKeySwing::OpenDoor(float dt)
{
	//Get Current Z rotation
	doorCurrentRotation = door->RelativeRotation.Yaw;

	addRotation = dt * 80;

	if (FMath::IsNearlyEqual(doorCurrentRotation, maxDegree, 1.5f))
	{
		Opening = false;
		Closing = false;
	}
	else if (Opening)
	{
		FRotator newRotation = FRotator(0.0f, addRotation, 0.0f);
		door->AddRelativeRotation(FQuat(newRotation), false, 0, ETeleportType::None);
	}
}

void ADoorKeySwing::CloseDoor(float dt)
{
	//Get Current Z rotation
	doorCurrentRotation = door->RelativeRotation.Yaw;

	if (doorCurrentRotation > 0)
	{
		addRotation = -dt * 80;
	}
	else
	{
		addRotation = dt * 80;
	}

	if (FMath::IsNearlyEqual(doorCurrentRotation, 0.0f, 1.5f))
	{
		Opening = false;
		Closing = false;
	}
	else if (Closing)
	{
		FRotator newRotation = FRotator(0.0f, addRotation, 0.0f);
		door->AddRelativeRotation(FQuat(newRotation), false, 0, ETeleportType::None);
	}
}

void ADoorKeySwing::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult &SweepResult)
{

	if (ADoorKey* keyInside = Cast<ADoorKey>(OtherActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("Door Key Inside"));
		isKeyInside = true;
	}
	
}

void ADoorKeySwing::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	// Other Actor is the actor that triggered the event. Check that is not ourself.  
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr))
	{
		UE_LOG(LogTemp, Warning, TEXT("Door Key Swing End Overlap Triggered"));
	}

	if (ADoorKey* keyInside = Cast<ADoorKey>(OtherActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("Door Key Outside"));
		isKeyInside = false;
	}
}

void ADoorKeySwing::OnActionKeyPressed(ADoorKey* key)
{
	// Ne key inside no fun
	if (isKeyInside)
	{
		// if key is NULL we have a problem
		if (key != NULL)
		{
			doorKey = key;
			keyLocation = doorKey->GetActorLocation();
		}
	}
}

bool ADoorKeySwing::KeyMovement(float dt)
{
	// No key -> No key movement 
	if (doorKey != NULL)
	{
		// Prepare key in front of the hole
		if (moveToStartingPosition)
		{
			// startPosition -> scene component, default position is in front of the door
			startPosition = keyStartPoint->GetComponentLocation();
			keyLocation = doorKey->GetActorLocation();

			// Get movement vector
			moveVector = startPosition - keyLocation;
			moveVector.Normalize();

			// Set key forward vector to point into the hole.
			// TODO:
			// Add shiny and smooth rotation so it will look perfect <3 
			doorKey->SetActorRotation(moveVector.Rotation());
			
			// How far key should move in one frame
			newKeyLocation = moveVector * moveSpeed * dt;
			newTransform = FTransform(newKeyLocation);

			// This function adds location to existiong one not replace it!
			doorKey->AddActorWorldTransform(newTransform);
			
			// Check if reached its destination
			if (FMath::IsNearlyEqual(doorKey->GetActorLocation().X, startPosition.X, 1.0f))
				if (FMath::IsNearlyEqual(doorKey->GetActorLocation().Y, startPosition.Y, 1.0f))
					if (FMath::IsNearlyEqual(doorKey->GetActorLocation().Z, startPosition.Z, 1.0f))
					{
						moveToEndPosition = true;
						moveToStartingPosition = false;
					}
		}

		if (moveToEndPosition)
		{
			// endPosition -> scene component, default position is inside the key hole
			endPosition = keyEndPoint->GetComponentLocation();
			keyLocation = doorKey->GetActorLocation();

			// Get movement vector
			moveVector = endPosition - keyLocation;
			moveVector.Normalize();

			// How far key should move in one frame
			newKeyLocation = moveVector * moveSpeed * dt;
			newTransform = FTransform(newKeyLocation);

			// This function adds location to existiong one not replace it!
			doorKey->AddActorWorldTransform(newTransform);

			// Check if reached its destination
			if (FMath::IsNearlyEqual(doorKey->GetActorLocation().X, endPosition.X, 1.0f))
				if (FMath::IsNearlyEqual(doorKey->GetActorLocation().Y, endPosition.Y, 1.0f))
					if (FMath::IsNearlyEqual(doorKey->GetActorLocation().Z, endPosition.Z, 1.0f))
					{
						moveToEndPosition = false;
						startRotate = true;
					}
		}

		if (startRotate)
		{
			// rotation value
			float addKeyRotation = rotationSpeed * dt;
			
			// need for test later if totalRotation = 360 stop rotation
			totalRotation += addKeyRotation;

			// Create rotator
			FRotator newKeyRotation = FRotator(0, 0, addKeyRotation);

			// Add rotator to rotation
			doorKey->AddActorWorldRotation(newKeyRotation);

			if (FMath::IsNearlyEqual(totalRotation, 360.0f, 1.0f))
			{
				startRotate = false;
				return true;
			}
		}
	}
	return false;
}