#include "SettingsWidget.h"
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"
#include "Components/CheckBox.h"
#include "Components/TextBlock.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

// Define static members
const FString USettingsWidget::InputSettingsSection = TEXT("FCJ.InputSettings");
const FString USettingsWidget::KeyMappingsSection = TEXT("FCJ.KeyMappings");

// Fixed default key mappings
const TMap<FString, FKey> USettingsWidget::DefaultKeyMappings = {
	{TEXT("MoveForward"), EKeys::W},
	{TEXT("MoveBackward"), EKeys::S},
	{TEXT("MoveLeft"), EKeys::A},
	{TEXT("MoveRight"), EKeys::D},
	{TEXT("Jump"), EKeys::SpaceBar},
	{TEXT("Action"), EKeys::LeftShift}
};

void USettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ResolutionComboBox)
	{
		ResolutionComboBox->OnSelectionChanged.AddDynamic(this, &USettingsWidget::OnResolutionChanged);
		PopulateResolutionOptions();
	}

	if (FullScreenButton)
	{
		FullScreenButton->OnClicked.AddDynamic(this, &USettingsWidget::OnFullScreenClicked);
	}

	if (WindowedButton)
	{
		WindowedButton->OnClicked.AddDynamic(this, &USettingsWidget::OnWindowedClicked);
	}

	if (BorderlessButton)
	{
		BorderlessButton->OnClicked.AddDynamic(this, &USettingsWidget::OnBorderlessClicked);
	}

	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &USettingsWidget::OnBackClicked);
	}

	// Input Settings
	if (MouseSensitivitySlider)
	{
		MouseSensitivitySlider->OnValueChanged.AddDynamic(this, &USettingsWidget::OnMouseSensitivityChanged);
		MouseSensitivitySlider->SetMinValue(0.1f);
		MouseSensitivitySlider->SetMaxValue(5.0f);
	}

	if (InvertMouseYCheckbox)
	{
		InvertMouseYCheckbox->OnCheckStateChanged.AddDynamic(this, &USettingsWidget::OnInvertMouseYChanged);
	}

	if (ZoomSpeedSlider)
	{
		ZoomSpeedSlider->OnValueChanged.AddDynamic(this, &USettingsWidget::OnZoomSpeedChanged);
		ZoomSpeedSlider->SetMinValue(10.0f);
		ZoomSpeedSlider->SetMaxValue(200.0f);
	}

	// Input Remapping Buttons
	if (MoveForwardKeyButton)
	{
		MoveForwardKeyButton->OnClicked.AddDynamic(this, &USettingsWidget::OnMoveForwardKeyClicked);
	}

	if (MoveBackwardKeyButton)
	{
		MoveBackwardKeyButton->OnClicked.AddDynamic(this, &USettingsWidget::OnMoveBackwardKeyClicked);
	}

	if (MoveLeftKeyButton)
	{
		MoveLeftKeyButton->OnClicked.AddDynamic(this, &USettingsWidget::OnMoveLeftKeyClicked);
	}

	if (MoveRightKeyButton)
	{
		MoveRightKeyButton->OnClicked.AddDynamic(this, &USettingsWidget::OnMoveRightKeyClicked);
	}

	if (JumpKeyButton)
	{
		JumpKeyButton->OnClicked.AddDynamic(this, &USettingsWidget::OnJumpKeyClicked);
	}

	if (ActionKeyButton)
	{
		ActionKeyButton->OnClicked.AddDynamic(this, &USettingsWidget::OnActionKeyClicked);
	}

	if (ResetToDefaultButton)
	{
		ResetToDefaultButton->OnClicked.AddDynamic(this, &USettingsWidget::OnResetToDefaultClicked);
	}

	if (ApplySettingsButton)
	{
		ApplySettingsButton->OnClicked.AddDynamic(this, &USettingsWidget::OnApplySettingsClicked);
	}

	// Initialize input mappings with default keys
	InitializeDefaultKeyMappings();

	bIsWaitingForKeyInput = false;
	CurrentRemappingAction = TEXT("");

	// Load saved settings
	LoadSettings();
}

void USettingsWidget::PopulateResolutionOptions()
{
	if (ResolutionComboBox)
	{
		ResolutionComboBox->ClearOptions();
		
		TArray<FString> ResolutionOptions = {
			TEXT("1920x1080"),
			TEXT("1680x1050"),
			TEXT("1600x900"),
			TEXT("1440x900"),
			TEXT("1366x768"),
			TEXT("1280x720"),
			TEXT("1024x768")
		};

		for (const FString& Resolution : ResolutionOptions)
		{
			ResolutionComboBox->AddOption(Resolution);
		}

		ResolutionComboBox->SetSelectedOption(TEXT("1920x1080"));
	}
}

void USettingsWidget::OnResolutionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (SelectionType != ESelectInfo::Direct)
	{
		ApplyResolution(SelectedItem);
	}
}

void USettingsWidget::OnFullScreenClicked()
{
	ApplyWindowMode(EWindowMode::Fullscreen);
}

void USettingsWidget::OnWindowedClicked()
{
	ApplyWindowMode(EWindowMode::Windowed);
}

void USettingsWidget::OnBorderlessClicked()
{
	ApplyWindowMode(EWindowMode::WindowedFullscreen);
}

void USettingsWidget::OnBackClicked()
{
	OnBackButtonClicked.Broadcast();
	SetVisibility(ESlateVisibility::Hidden);
}

void USettingsWidget::ApplyResolution(const FString& Resolution)
{
	TArray<FString> ResolutionParts;
	Resolution.ParseIntoArray(ResolutionParts, TEXT("x"), true);

	if (ResolutionParts.Num() == 2)
	{
		int32 Width = FCString::Atoi(*ResolutionParts[0]);
		int32 Height = FCString::Atoi(*ResolutionParts[1]);

		CachedResolution = FIntPoint(Width, Height);
		
		// Apply immediately using GameUserSettings
		UGameUserSettings* GameUserSettings = UGameUserSettings::GetGameUserSettings();
		if (GameUserSettings)
		{
			GameUserSettings->SetScreenResolution(CachedResolution);
			GameUserSettings->ApplyResolutionSettings(false);
		}
	}
}

void USettingsWidget::ApplyWindowMode(EWindowMode::Type WindowMode)
{
	if (UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport())
	{
		ViewportClient->GetWindow()->SetWindowMode(WindowMode);
	}
	CachedWindowMode = WindowMode;
}

// Input Settings Functions
void USettingsWidget::OnMouseSensitivityChanged(float Value)
{
	CachedMouseSensitivity = Value;
	if (MouseSensitivityValue)
	{
		FNumberFormattingOptions NumberFormat = FNumberFormattingOptions::DefaultWithGrouping();
		NumberFormat.SetMaximumFractionalDigits(2);
		MouseSensitivityValue->SetText(FText::AsNumber(Value, &NumberFormat));
	}
	
	// Cache the value for saving later
}

void USettingsWidget::OnInvertMouseYChanged(bool bIsChecked)
{
	bCachedInvertMouseY = bIsChecked;
	
	// Cache the value for saving later
}

void USettingsWidget::OnZoomSpeedChanged(float Value)
{
	CachedZoomSpeed = Value;
	if (ZoomSpeedValue)
	{
		FNumberFormattingOptions NumberFormat = FNumberFormattingOptions::DefaultWithGrouping();
		NumberFormat.SetMaximumFractionalDigits(0);
		ZoomSpeedValue->SetText(FText::AsNumber(Value, &NumberFormat));
	}
	
	// Cache the value for saving later
}

// Input Remapping Functions
void USettingsWidget::OnMoveForwardKeyClicked()
{
	StartKeyRemapping(TEXT("MoveForward"));
}

void USettingsWidget::OnMoveBackwardKeyClicked()
{
	StartKeyRemapping(TEXT("MoveBackward"));
}

void USettingsWidget::OnMoveLeftKeyClicked()
{
	StartKeyRemapping(TEXT("MoveLeft"));
}

void USettingsWidget::OnMoveRightKeyClicked()
{
	StartKeyRemapping(TEXT("MoveRight"));
}

void USettingsWidget::OnJumpKeyClicked()
{
	StartKeyRemapping(TEXT("Jump"));
}

void USettingsWidget::OnActionKeyClicked()
{
	StartKeyRemapping(TEXT("Action"));
}

void USettingsWidget::OnResetToDefaultClicked()
{
	ResetInputMappingsToDefault();
	
	// Update UI immediately with default values
	UpdateButtonText(MoveForwardKeyText, DefaultKeyMappings[TEXT("MoveForward")]);
	UpdateButtonText(MoveBackwardKeyText, DefaultKeyMappings[TEXT("MoveBackward")]);
	UpdateButtonText(MoveLeftKeyText, DefaultKeyMappings[TEXT("MoveLeft")]);
	UpdateButtonText(MoveRightKeyText, DefaultKeyMappings[TEXT("MoveRight")]);
	UpdateButtonText(JumpKeyText, DefaultKeyMappings[TEXT("Jump")]);
	UpdateButtonText(ActionKeyText, DefaultKeyMappings[TEXT("Action")]);
	
	// Save the reset values to config
	SaveSettings();
}

void USettingsWidget::OnApplySettingsClicked()
{
	SaveSettings();
	ApplyInputSettings();
}

// Settings Management
void USettingsWidget::LoadSettings()
{
	UGameUserSettings* GameUserSettings = UGameUserSettings::GetGameUserSettings();
	if (GameUserSettings)
	{
		// Load display settings
		CachedResolution = GameUserSettings->GetScreenResolution();
		CachedWindowMode = GameUserSettings->GetFullscreenMode();
		
		// Set resolution combo box
		if (ResolutionComboBox)
		{
			FString ResolutionString = FString::Printf(TEXT("%dx%d"), CachedResolution.X, CachedResolution.Y);
			ResolutionComboBox->SetSelectedOption(ResolutionString);
		}
	}

	// Load input settings from config file
	TMap<FString, FString> KeyMappingsStrings;
	LoadInputSettingsFromConfig(CachedMouseSensitivity, bCachedInvertMouseY, CachedZoomSpeed, KeyMappingsStrings);

	// Convert string key mappings back to FKey objects
	for (const auto& Pair : KeyMappingsStrings)
	{
		if (InputMappings.Contains(Pair.Key))
		{
			FKey LoadedKey(*Pair.Value);
			if (LoadedKey.IsValid())
			{
				InputMappings[Pair.Key].CurrentKey = LoadedKey;
			}
		}
	}

	// Update UI elements
	if (MouseSensitivitySlider)
	{
		MouseSensitivitySlider->SetValue(CachedMouseSensitivity);
	}
	if (InvertMouseYCheckbox)
	{
		InvertMouseYCheckbox->SetIsChecked(bCachedInvertMouseY);
	}
	if (ZoomSpeedSlider)
	{
		ZoomSpeedSlider->SetValue(CachedZoomSpeed);
	}

	// Update button texts with current key mappings
	UpdateButtonText(MoveForwardKeyText, InputMappings.FindRef(TEXT("MoveForward")).CurrentKey);
	UpdateButtonText(MoveBackwardKeyText, InputMappings.FindRef(TEXT("MoveBackward")).CurrentKey);
	UpdateButtonText(MoveLeftKeyText, InputMappings.FindRef(TEXT("MoveLeft")).CurrentKey);
	UpdateButtonText(MoveRightKeyText, InputMappings.FindRef(TEXT("MoveRight")).CurrentKey);
	UpdateButtonText(JumpKeyText, InputMappings.FindRef(TEXT("Jump")).CurrentKey);
	UpdateButtonText(ActionKeyText, InputMappings.FindRef(TEXT("Action")).CurrentKey);

	// Update value text displays
	OnMouseSensitivityChanged(CachedMouseSensitivity);
	OnZoomSpeedChanged(CachedZoomSpeed);
}

void USettingsWidget::SaveSettings()
{
	UGameUserSettings* GameUserSettings = UGameUserSettings::GetGameUserSettings();
	if (GameUserSettings)
	{
		// Save display settings
		GameUserSettings->SetScreenResolution(CachedResolution);
		GameUserSettings->SetFullscreenMode(CachedWindowMode);
		GameUserSettings->ApplySettings(false);
		GameUserSettings->SaveSettings();
	}

	// Save input settings to config file
	TMap<FString, FString> KeyMappingsStrings = GetCurrentKeyMappingsAsStrings();
	SaveInputSettingsToConfig(CachedMouseSensitivity, bCachedInvertMouseY, CachedZoomSpeed, KeyMappingsStrings);
}

void USettingsWidget::ApplyInputSettings()
{
	// Input settings are now saved to config and will be loaded by MultiPlayerController on BeginPlay
	// No need to apply directly here since we're in the menu
}

// Input Remapping
void USettingsWidget::StartKeyRemapping(const FString& ActionName)
{
	// Cancel any existing key remapping first
	if (bIsWaitingForKeyInput && !CurrentRemappingAction.IsEmpty())
	{
		// Restore the previous action's button text
		RestoreButtonText(CurrentRemappingAction);
	}
	
	CurrentRemappingAction = ActionName;
	bIsWaitingForKeyInput = true;

	// Update button text to show waiting state
	UTextBlock* TextWidget = nullptr;
	if (ActionName == TEXT("MoveForward")) TextWidget = MoveForwardKeyText;
	else if (ActionName == TEXT("MoveBackward")) TextWidget = MoveBackwardKeyText;
	else if (ActionName == TEXT("MoveLeft")) TextWidget = MoveLeftKeyText;
	else if (ActionName == TEXT("MoveRight")) TextWidget = MoveRightKeyText;
	else if (ActionName == TEXT("Jump")) TextWidget = JumpKeyText;
	else if (ActionName == TEXT("Action")) TextWidget = ActionKeyText;

	if (TextWidget)
	{
		FText ButtonText = FText::FromString(TEXT("Press any key..."));
		TextWidget->SetText(ButtonText);
	}
}

void USettingsWidget::UpdateKeyBinding(const FString& ActionName, const FKey& NewKey)
{
	if (InputMappings.Contains(ActionName))
	{
		// Check if this key is already used by another action
		FString ExistingAction = FindActionByKey(NewKey);
		if (!ExistingAction.IsEmpty() && ExistingAction != ActionName)
		{
			// Swap keys: give the existing action our current key
			FKey OldKey = InputMappings[ActionName].CurrentKey;
			InputMappings[ExistingAction].CurrentKey = OldKey;
			
			// Update the existing action's button text
			if (ExistingAction == TEXT("MoveForward"))
			{
				UpdateButtonText(MoveForwardKeyText, OldKey);
			}
			else if (ExistingAction == TEXT("MoveBackward"))
			{
				UpdateButtonText(MoveBackwardKeyText, OldKey);
			}
			else if (ExistingAction == TEXT("MoveLeft"))
			{
				UpdateButtonText(MoveLeftKeyText, OldKey);
			}
			else if (ExistingAction == TEXT("MoveRight"))
			{
				UpdateButtonText(MoveRightKeyText, OldKey);
			}
			else if (ExistingAction == TEXT("Jump"))
			{
				UpdateButtonText(JumpKeyText, OldKey);
			}
			else if (ExistingAction == TEXT("Action"))
			{
				UpdateButtonText(ActionKeyText, OldKey);
			}
			
			UE_LOG(LogTemp, Warning, TEXT("Key swap: %s got %s, %s got %s"), *ActionName, *NewKey.ToString(), *ExistingAction, *OldKey.ToString());
		}
		
		// Set the new key for the current action
		InputMappings[ActionName].CurrentKey = NewKey;
		
		// Update button text for the current action
		if (ActionName == TEXT("MoveForward"))
		{
			UpdateButtonText(MoveForwardKeyText, NewKey);
		}
		else if (ActionName == TEXT("MoveBackward"))
		{
			UpdateButtonText(MoveBackwardKeyText, NewKey);
		}
		else if (ActionName == TEXT("MoveLeft"))
		{
			UpdateButtonText(MoveLeftKeyText, NewKey);
		}
		else if (ActionName == TEXT("MoveRight"))
		{
			UpdateButtonText(MoveRightKeyText, NewKey);
		}
		else if (ActionName == TEXT("Jump"))
		{
			UpdateButtonText(JumpKeyText, NewKey);
		}
		else if (ActionName == TEXT("Action"))
		{
			UpdateButtonText(ActionKeyText, NewKey);
		}
	}
}

void USettingsWidget::UpdateButtonText(UTextBlock* TextWidget, const FKey& Key)
{
	if (TextWidget && Key.IsValid())
	{
		FText ButtonText = Key.GetDisplayName();
		TextWidget->SetText(ButtonText);
	}
}

void USettingsWidget::ResetInputMappingsToDefault()
{
	for (auto& Mapping : InputMappings)
	{
		// Use fixed default values
		if (DefaultKeyMappings.Contains(Mapping.Key))
		{
			Mapping.Value.CurrentKey = DefaultKeyMappings[Mapping.Key];
			Mapping.Value.DefaultKey = DefaultKeyMappings[Mapping.Key];
		}
	}
}

// Key capture
FReply USettingsWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (bIsWaitingForKeyInput && !CurrentRemappingAction.IsEmpty())
	{
		FKey PressedKey = InKeyEvent.GetKey();
		
		// Ignore certain keys
		if (PressedKey != EKeys::Escape && PressedKey != EKeys::Enter)
		{
			UpdateKeyBinding(CurrentRemappingAction, PressedKey);
			bIsWaitingForKeyInput = false;
			CurrentRemappingAction = TEXT("");
			return FReply::Handled();
		}
		else if (PressedKey == EKeys::Escape)
		{
			// Cancel remapping
			RestoreButtonText(CurrentRemappingAction);
			bIsWaitingForKeyInput = false;
			CurrentRemappingAction = TEXT("");
			return FReply::Handled();
		}
	}
	
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply USettingsWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bIsWaitingForKeyInput && !CurrentRemappingAction.IsEmpty())
	{
		FKey PressedKey = InMouseEvent.GetEffectingButton();
		
		UpdateKeyBinding(CurrentRemappingAction, PressedKey);
		bIsWaitingForKeyInput = false;
		CurrentRemappingAction = TEXT("");
		return FReply::Handled();
	}
	
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

// Public functions
void USettingsWidget::RefreshCurrentSettings()
{
	LoadSettings();
}

void USettingsWidget::RevertToSavedSettings()
{
	LoadSettings();
}

void USettingsWidget::InitializeDefaultKeyMappings()
{
	// Clear existing mappings
	InputMappings.Empty();
	
	// Use fixed default key mappings
	for (const auto& DefaultPair : DefaultKeyMappings)
	{
		InputMappings.Add(DefaultPair.Key, FInputRemapData(DefaultPair.Key, DefaultPair.Value));
	}
}

TMap<FString, FString> USettingsWidget::GetCurrentKeyMappingsAsStrings() const
{
	TMap<FString, FString> StringMappings;
	
	for (const auto& Pair : InputMappings)
	{
		StringMappings.Add(Pair.Key, Pair.Value.CurrentKey.ToString());
	}
	
	return StringMappings;
}

// Static functions for saving/loading settings
void USettingsWidget::SaveInputSettingsToConfig(float MouseSensitivity, bool bInvertMouseY, float ZoomSpeed, const TMap<FString, FString>& KeyMappings)
{
	if (GConfig)
	{
		// Save mouse sensitivity
		GConfig->SetFloat(*InputSettingsSection, TEXT("MouseSensitivity"), MouseSensitivity, GGameUserSettingsIni);
		
		// Save invert mouse Y
		GConfig->SetBool(*InputSettingsSection, TEXT("InvertMouseY"), bInvertMouseY, GGameUserSettingsIni);
		
		// Save zoom speed
		GConfig->SetFloat(*InputSettingsSection, TEXT("ZoomSpeed"), ZoomSpeed, GGameUserSettingsIni);
		
		// Save key mappings
		for (const auto& Pair : KeyMappings)
		{
			GConfig->SetString(*KeyMappingsSection, *Pair.Key, *Pair.Value, GGameUserSettingsIni);
		}
		
		// Flush the config to disk
		GConfig->Flush(false, GGameUserSettingsIni);
	}
}

void USettingsWidget::LoadInputSettingsFromConfig(float& OutMouseSensitivity, bool& OutInvertMouseY, float& OutZoomSpeed, TMap<FString, FString>& OutKeyMappings)
{
	// Set defaults
	OutMouseSensitivity = 1.0f;
	OutInvertMouseY = false;
	OutZoomSpeed = 50.0f;
	OutKeyMappings.Empty();
	
	if (GConfig)
	{
		// Load mouse sensitivity
		GConfig->GetFloat(*InputSettingsSection, TEXT("MouseSensitivity"), OutMouseSensitivity, GGameUserSettingsIni);
		
		// Load invert mouse Y
		GConfig->GetBool(*InputSettingsSection, TEXT("InvertMouseY"), OutInvertMouseY, GGameUserSettingsIni);
		
		// Load zoom speed
		GConfig->GetFloat(*InputSettingsSection, TEXT("ZoomSpeed"), OutZoomSpeed, GGameUserSettingsIni);
		
		// Load key mappings for individual keys
		TArray<FString> ActionNames = {TEXT("MoveForward"), TEXT("MoveBackward"), TEXT("MoveLeft"), TEXT("MoveRight"), TEXT("Jump"), TEXT("Action")};
		for (const FString& ActionName : ActionNames)
		{
			FString KeyString;
			if (GConfig->GetString(*KeyMappingsSection, *ActionName, KeyString, GGameUserSettingsIni))
			{
				OutKeyMappings.Add(ActionName, KeyString);
			}
		}
	}
}

FString USettingsWidget::GetConfigFilePath()
{
	// Return the path to the GameUserSettings.ini file where our settings are saved
	FString ConfigPath = GGameUserSettingsIni;
	if (ConfigPath.IsEmpty())
	{
		// Fallback to default path construction
		FString SavedDir = FPaths::ProjectSavedDir();
		ConfigPath = FPaths::Combine(SavedDir, TEXT("Config"), FPlatformProperties::PlatformName(), TEXT("GameUserSettings.ini"));
	}
	
	// Log the path for debugging
	UE_LOG(LogTemp, Warning, TEXT("Settings config file location: %s"), *ConfigPath);
	UE_LOG(LogTemp, Warning, TEXT("Settings sections: [%s] and [%s]"), *InputSettingsSection, *KeyMappingsSection);
	
	return ConfigPath;
}

void USettingsWidget::RestoreButtonText(const FString& ActionName)
{
	if (InputMappings.Contains(ActionName))
	{
		FKey CurrentKey = InputMappings[ActionName].CurrentKey;
		
		if (ActionName == TEXT("MoveForward"))
		{
			UpdateButtonText(MoveForwardKeyText, CurrentKey);
		}
		else if (ActionName == TEXT("MoveBackward"))
		{
			UpdateButtonText(MoveBackwardKeyText, CurrentKey);
		}
		else if (ActionName == TEXT("MoveLeft"))
		{
			UpdateButtonText(MoveLeftKeyText, CurrentKey);
		}
		else if (ActionName == TEXT("MoveRight"))
		{
			UpdateButtonText(MoveRightKeyText, CurrentKey);
		}
		else if (ActionName == TEXT("Jump"))
		{
			UpdateButtonText(JumpKeyText, CurrentKey);
		}
		else if (ActionName == TEXT("Action"))
		{
			UpdateButtonText(ActionKeyText, CurrentKey);
		}
	}
}

FString USettingsWidget::FindActionByKey(const FKey& Key) const
{
	for (const auto& Pair : InputMappings)
	{
		if (Pair.Value.CurrentKey == Key)
		{
			return Pair.Key;
		}
	}
	return FString();
}