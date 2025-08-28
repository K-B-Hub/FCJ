
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LocalPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class FCJ_API ALocalPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    ALocalPlayerController();

protected:
    virtual void OnPossess(APawn* aPawn) override;
    virtual void SetupInputComponent() override;

private:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInputAction> MoveFrontAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInputAction> JumpFrontAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInputAction> MoveBackAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInputAction> JumpBackAction;

    void MoveFront(const FInputActionValue& Value);
    void JumpFront();

    void MoveBack(const FInputActionValue& Value);
    void JumpBack();

    UPROPERTY()
    TObjectPtr<class ALocalPlayerCharacter> PlayerCharacter;
};
