#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameUserSettings.h"
#include "SettingsWidget.generated.h"

class UButton;
class UComboBoxString;
class USlider;
class UCheckBox;
class UTextBlock;
class UInputAction;
class UInputMappingContext;
class AMultiPlayerController;

USTRUCT(BlueprintType)
struct FInputRemapData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ActionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FKey DefaultKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FKey CurrentKey;

	FInputRemapData()
	{
		ActionName = TEXT("");
		DefaultKey = EKeys::Invalid;
		CurrentKey = EKeys::Invalid;
	}

	FInputRemapData(const FString& InActionName, const FKey& InDefaultKey)
		: ActionName(InActionName), DefaultKey(InDefaultKey), CurrentKey(InDefaultKey)
	{
	}
};

UCLASS(Blueprintable)
class FCJ_API USettingsWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UComboBoxString* ResolutionComboBox;

	UPROPERTY(meta = (BindWidget))
	UButton* FullScreenButton;

	UPROPERTY(meta = (BindWidget))
	UButton* WindowedButton;

	UPROPERTY(meta = (BindWidget))
	UButton* BorderlessButton;

	UPROPERTY(meta = (BindWidget))
	UButton* BackButton;

	// Input Settings
	UPROPERTY(meta = (BindWidget))
	USlider* MouseSensitivitySlider;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MouseSensitivityValue;

	UPROPERTY(meta = (BindWidget))
	UCheckBox* InvertMouseYCheckbox;

	UPROPERTY(meta = (BindWidget))
	USlider* ZoomSpeedSlider;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ZoomSpeedValue;

	// Input Remapping Buttons - IA_Move (2D Vector with W,A,S,D)
	UPROPERTY(meta = (BindWidget))
	UButton* MoveForwardKeyButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MoveForwardKeyText;

	UPROPERTY(meta = (BindWidget))
	UButton* MoveBackwardKeyButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MoveBackwardKeyText;

	UPROPERTY(meta = (BindWidget))
	UButton* MoveLeftKeyButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MoveLeftKeyText;

	UPROPERTY(meta = (BindWidget))
	UButton* MoveRightKeyButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MoveRightKeyText;

	// IA_Jump
	UPROPERTY(meta = (BindWidget))
	UButton* JumpKeyButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* JumpKeyText;

	// IA_Action
	UPROPERTY(meta = (BindWidget))
	UButton* ActionKeyButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ActionKeyText;

	UPROPERTY(meta = (BindWidget))
	UButton* ResetToDefaultButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ApplySettingsButton;

private:
	UFUNCTION()
	void OnResolutionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnFullScreenClicked();

	UFUNCTION()
	void OnWindowedClicked();

	UFUNCTION()
	void OnBorderlessClicked();

	UFUNCTION()
	void OnBackClicked();

	// Input Settings Functions
	UFUNCTION()
	void OnMouseSensitivityChanged(float Value);

	UFUNCTION()
	void OnInvertMouseYChanged(bool bIsChecked);

	UFUNCTION()
	void OnZoomSpeedChanged(float Value);

	// Input Remapping Functions
	UFUNCTION()
	void OnMoveForwardKeyClicked();

	UFUNCTION()
	void OnMoveBackwardKeyClicked();

	UFUNCTION()
	void OnMoveLeftKeyClicked();

	UFUNCTION()
	void OnMoveRightKeyClicked();

	UFUNCTION()
	void OnJumpKeyClicked();

	UFUNCTION()
	void OnActionKeyClicked();

	UFUNCTION()
	void OnResetToDefaultClicked();

	UFUNCTION()
	void OnApplySettingsClicked();

	void PopulateResolutionOptions();
	void ApplyResolution(const FString& Resolution);
	void ApplyWindowMode(EWindowMode::Type WindowMode);

	// Settings Management
	void LoadSettings();
	void SaveSettings();
	void ApplyInputSettings();

	// Input Remapping
	void StartKeyRemapping(const FString& ActionName);
	void UpdateKeyBinding(const FString& ActionName, const FKey& NewKey);
	void UpdateButtonText(UTextBlock* TextWidget, const FKey& Key);
	void ResetInputMappingsToDefault();
	void InitializeInputMappingsFromIMC();
	UInputAction* GetInputActionForName(const FString& ActionName);

	// Key capture
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual bool SupportsKeyboardFocus() const { return true; }

	// Member variables
	TMap<FString, FInputRemapData> InputMappings;
	FString CurrentRemappingAction;
	bool bIsWaitingForKeyInput;

	// Cached settings
	float CachedMouseSensitivity;
	bool bCachedInvertMouseY;
	float CachedZoomSpeed;
	FIntPoint CachedResolution;
	EWindowMode::Type CachedWindowMode;

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBackButtonClicked);
	UPROPERTY(BlueprintAssignable)
	FOnBackButtonClicked OnBackButtonClicked;

	// Public functions for external access
	UFUNCTION(BlueprintCallable)
	void RefreshCurrentSettings();

	UFUNCTION(BlueprintCallable)
	void RevertToSavedSettings();

	// Static functions for saving/loading settings (can be called from any controller)
	UFUNCTION(BlueprintCallable, Category = "Settings")
	static void SaveInputSettingsToConfig(float MouseSensitivity, bool bInvertMouseY, float ZoomSpeed, const TMap<FString, FString>& KeyMappings);

	UFUNCTION(BlueprintCallable, Category = "Settings")
	static void LoadInputSettingsFromConfig(float& OutMouseSensitivity, bool& OutInvertMouseY, float& OutZoomSpeed, TMap<FString, FString>& OutKeyMappings);

	// Debug function to get config file path
	UFUNCTION(BlueprintCallable, Category = "Settings")
	static FString GetConfigFilePath();

private:
	// Config section names
	static const FString InputSettingsSection;
	static const FString KeyMappingsSection;
	
	// Fixed default values
	static const TMap<FString, FKey> DefaultKeyMappings;

	// Helper functions
	TMap<FString, FString> GetCurrentKeyMappingsAsStrings() const;
	void InitializeDefaultKeyMappings();
	void RestoreButtonText(const FString& ActionName);
	FString FindActionByKey(const FKey& Key) const;
};