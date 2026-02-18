// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/GCFActivatableWidget.h"

#include "Editor/WidgetCompilerLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GCFActivatableWidget)

#define LOCTEXT_NAMESPACE "GCF"

UGCFActivatableWidget::UGCFActivatableWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}

TOptional<FUIInputConfig> UGCFActivatableWidget::GetDesiredInputConfig() const
{
	switch (InputConfig) {
		case EGCFWidgetInputMode::GameAndMenu:
			return FUIInputConfig(ECommonInputMode::All, GameMouseCaptureMode);
		case EGCFWidgetInputMode::Game:
			return FUIInputConfig(ECommonInputMode::Game, GameMouseCaptureMode);
		case EGCFWidgetInputMode::Menu:
			return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture);
		case EGCFWidgetInputMode::Default:
		default:
			return TOptional<FUIInputConfig>();
	}
}

#if WITH_EDITOR

void UGCFActivatableWidget::ValidateCompiledWidgetTree(const UWidgetTree& BlueprintWidgetTree, class IWidgetCompilerLog& CompileLog) const
{
	Super::ValidateCompiledWidgetTree(BlueprintWidgetTree, CompileLog);

	if (!GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(UGCFActivatableWidget, BP_GetDesiredFocusTarget))) {
		if (GetParentNativeClass(GetClass()) == UGCFActivatableWidget::StaticClass()) {
			CompileLog.Warning(LOCTEXT("ValidateGetDesiredFocusTarget_Warning", "GetDesiredFocusTarget wasn't implemented, you're going to have trouble using gamepads on this screen."));
		} else {
			//TODO - Note for now, because we can't guarantee it isn't implemented in a native subclass of this one.
			CompileLog.Note(LOCTEXT("ValidateGetDesiredFocusTarget_Note", "GetDesiredFocusTarget wasn't implemented, you're going to have trouble using gamepads on this screen.  If it was implemented in the native base class you can ignore this message."));
		}
	}
}

#endif

#undef LOCTEXT_NAMESPACE
