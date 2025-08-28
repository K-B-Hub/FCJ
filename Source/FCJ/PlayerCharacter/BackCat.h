
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BackCat.generated.h"

UCLASS()
class FCJ_API ABackCat : public ACharacter
{
    GENERATED_BODY()

public:
    ABackCat();

protected:
    virtual void BeginPlay() override;

public: 
    virtual void Tick(float DeltaTime) override;
};
