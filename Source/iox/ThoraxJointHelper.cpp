// Fill out your copyright notice in the Description page of Project Settings.

#include "ThoraxJointHelper.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Widget.h"
#include "UDepthGraphWidget.h"

const TMap<FString, EThoraxJointRole>& GetThoraxJointRoleDictionary()
{
    static const TMap<FString, EThoraxJointRole> Dictionary = {
        {TEXT("neck_1_joint"),          EThoraxJointRole::Torso},
        {TEXT("root"),                  EThoraxJointRole::Torso},
        {TEXT("left_shoulder_1_joint"), EThoraxJointRole::LeftShoulder},
        {TEXT("right_shoulder_1_joint"),EThoraxJointRole::RightShoulder},
        {TEXT("left_upleg_joint"),      EThoraxJointRole::LeftHip},
        {TEXT("right_upleg_joint"),     EThoraxJointRole::RightHip}
    };
    return Dictionary;
}

EThoraxJointRole ResolveThoraxJointRole(const FString& RawName)
{
    const FString NameLower = RawName.ToLower();
    if (const EThoraxJointRole* Role = GetThoraxJointRoleDictionary().Find(NameLower))
    {
        return *Role;
    }
    return EThoraxJointRole::Unknown;
}

UUDepthGraphWidget* FindDepthGraphWidget(UUserWidget* RootWidget)
{
    if (!RootWidget)
    {
        return nullptr;
    }

    if (UUDepthGraphWidget* DirectGraphWidget = Cast<UUDepthGraphWidget>(RootWidget))
    {
        return DirectGraphWidget;
    }

    UWidgetTree* Tree = RootWidget->WidgetTree;
    if (!Tree)
    {
        return nullptr;
    }

    TArray<UWidget*> AllWidgets;
    Tree->GetAllWidgets(AllWidgets);
    for (UWidget* Widget : AllWidgets)
    {
        if (UUDepthGraphWidget* GraphWidget = Cast<UUDepthGraphWidget>(Widget))
        {
            return GraphWidget;
        }
    }

    return nullptr;
}
