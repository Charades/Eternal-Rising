// Fill out your copyright notice in the Description page of Project Settings.


#include "ServerList.h"
#include "ServerEntry.h"

void UServerList::OnSelectionChangedInternal(UObject* FirstSelectedItem)
{
    Super::OnSelectionChangedInternal(FirstSelectedItem);

    if (!FirstSelectedItem)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("No item selected."));
        return;
    }

    USteamServerWrapper* Entry = Cast<USteamServerWrapper>(FirstSelectedItem);
    if (Entry)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Server entry cast successful."));

        // Reset the border color of the previously selected item
        if (PreviousSelectedWidget)
        {
            if (UServerEntry* PreviousEntryWidget = Cast<UServerEntry>(PreviousSelectedWidget))
            {
                const int32 Index = GetIndexForItem(PreviousEntryWidget->GetListItem());
                if (Index % 2 == 0)
                {
                    // Even index: Dark color
                    PreviousEntryWidget->SetBorderColor(FLinearColor(0.015f, 0.015f, 0.015f, 1.0f));
                }
                else
                {
                    // Odd index: Light color
                    PreviousEntryWidget->SetBorderColor(FLinearColor(0.025f, 0.025f, 0.025f, 1.0f));
                }
            }
        }

        // Get the current selected widget and update its color
        UUserWidget* SelectedWidget = GetEntryWidgetFromItem(FirstSelectedItem);
        if (UServerEntry* ServerEntryWidget = Cast<UServerEntry>(SelectedWidget))
        {
            ServerEntryWidget->SetBorderColor(FLinearColor(0.670588f, 0.313726f, 0.0f, 1.0f)); // Highlight the selected item
            PreviousSelectedWidget = SelectedWidget; // Store reference for next deselection
        }

        OnServerListItemSelectionChanged.Broadcast(Entry, true);
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Server entry cast failed."));
        OnServerListItemSelectionChanged.Broadcast(nullptr, false);
    }
}

void UServerList::ClearSelections()
{
    if (PreviousSelectedWidget)
    {
        if (UServerEntry* PreviousEntryWidget = Cast<UServerEntry>(PreviousSelectedWidget))
        {
            const int32 Index = GetIndexForItem(PreviousEntryWidget->GetListItem());
            if (Index % 2 == 0)
            {
                // Even index: Dark color
                PreviousEntryWidget->SetBorderColor(FLinearColor(0.015f, 0.015f, 0.015f, 1.0f));
            }
            else
            {
                // Odd index: Light color
                PreviousEntryWidget->SetBorderColor(FLinearColor(0.025f, 0.025f, 0.025f, 1.0f));
            }
        }
    }

    PreviousSelectedWidget = nullptr;
}