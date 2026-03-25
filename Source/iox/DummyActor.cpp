#include "DummyActor.h"

ADummyActor::ADummyActor()
{
 	PrimaryActorTick.bCanEverTick = true;

}

void ADummyActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void ADummyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

