// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "UnrealTournament.h"
#include "UTHUDWidget_WeaponBar.h"
#include "UTWeapon.h"
#include "UTHUDWidgetMessage.h"

UUTHUDWidget_WeaponBar::UUTHUDWidget_WeaponBar(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Position=FVector2D(0.0f, -90.0f);
	Size=FVector2D(0,0);
	ScreenPosition=FVector2D(1.0f, 1.0f);
	Origin=FVector2D(1.0f,1.0f);

	SelectedCellScale=1.1;
	SelectedAnimRate=0.3;
	CellWidth = 145;
}

void UUTHUDWidget_WeaponBar::InitializeWidget(AUTHUD* Hud)
{
	LastGroup = -1;
	LastGroupSlot = -1.0;

	if (CellBackground.Num() < 2)
	{
		CellBackground.SetNumZeroed(2);
	}
	if (CellBorders.Num() < 2)
	{
		CellBorders.SetNumZeroed(2);
	}
	if (GroupHeaderCap.Num() < 2)
	{
		GroupHeaderCap.SetNumZeroed(2);
	}
	if (GroupSpacerBorder.Num() < 2)
	{
		GroupSpacerBorder.SetNumZeroed(2);
	}
	if (HeaderTab.Num() < 2)
	{
		HeaderTab.SetNumZeroed(2);
	}

	Super::InitializeWidget(Hud);
}

/**
 *	We aren't going tor use DrawAllRenderObjects.  Instead we are going to have a nice little custom bit of drawing based on what weapon gropup this 
 *  is.
 **/
void UUTHUDWidget_WeaponBar::Draw_Implementation(float DeltaTime)
{
	TArray<FWeaponGroup> WeaponGroups;

	if (UTCharacterOwner == NULL) return; // Don't draw without a character

	// Handle fading out.
	if (FadeTimer > 0.0f)
	{
		FadeTimer -= DeltaTime;
	}
	else 
	{
		if (InactiveOpacity != UTHUDOwner->HUDWidgetWeaponbarInactiveOpacity)
		{
			float Delta = (1.0 - UTHUDOwner->HUDWidgetWeaponbarInactiveOpacity) * DeltaTime;	// 1 second fade
			if (InactiveOpacity < UTHUDOwner->HUDWidgetWeaponbarInactiveOpacity) 
			{
				InactiveOpacity = FMath::Clamp<float>(InactiveOpacity + Delta, 0.0, UTHUDOwner->HUDWidgetWeaponbarInactiveOpacity);
			}
			else
			{
				InactiveOpacity = FMath::Clamp<float>(InactiveOpacity - Delta, UTHUDOwner->HUDWidgetWeaponbarInactiveOpacity, 1.0);
			}
		}

		if (InactiveIconOpacity != UTHUDOwner->HUDWidgetWeaponBarInactiveIconOpacity)
		{
			float Delta = (1.0 - UTHUDOwner->HUDWidgetWeaponBarInactiveIconOpacity) * DeltaTime;	// 1 second fade
			if (InactiveIconOpacity < UTHUDOwner->HUDWidgetWeaponBarInactiveIconOpacity) 
			{
				InactiveIconOpacity = FMath::Clamp<float>(InactiveIconOpacity + Delta, 0.0, UTHUDOwner->HUDWidgetWeaponBarInactiveIconOpacity);
			}
			else
			{
				InactiveIconOpacity = FMath::Clamp<float>(InactiveIconOpacity - Delta, UTHUDOwner->HUDWidgetWeaponBarInactiveIconOpacity, 1.0);
			}
		}
	}

	AUTWeapon* SelectedWeapon = UTCharacterOwner->GetPendingWeapon();
	if (SelectedWeapon == NULL)
	{
		SelectedWeapon = UTCharacterOwner->GetWeapon();
	}
	else
	{
		if (!WeaponNameText.Text.EqualTo(SelectedWeapon->DisplayName))
		{
			WeaponNameText.Text = SelectedWeapon->DisplayName;
			WeaponNameText.RenderOpacity = 1.0;
			WeaponNameDisplayTimer = WeaponNameDisplayTime;
		}
	}

	if (SelectedWeapon)
	{
		if (SelectedWeapon->Group != LastGroup || SelectedWeapon->GroupSlot != LastGroupSlot)
		{
			// Weapon has changed.. set everything up.
			InactiveOpacity = 1.0;
			InactiveIconOpacity = 1.0;
			FadeTimer = 1.0;

			LastGroup = SelectedWeapon->Group;
			LastGroupSlot = SelectedWeapon->GroupSlot;
		}
	}

	CollectWeaponData(WeaponGroups, DeltaTime);

	if (WeaponGroups.Num() > 0)
	{
		// Draw the Weapon Groups
		float YPosition = 0.0;
		int32 SelectedGroup = SelectedWeapon ? SelectedWeapon->Group : -1;

		for (int32 GroupIdx = 0; GroupIdx < WeaponGroups.Num(); GroupIdx++)
		{
			// We have no allied all of the animation and we know the biggest anim scale, so we can figure out how wide this group should be.
			float Y2 = YPosition;
			float TextXPosition = 0;
			bool bSelectedGroup = false;
			if (WeaponGroups[GroupIdx].WeaponsInGroup.Num() > 0)
			{
				// Draw the elements.
				for (int32 WeapIdx = WeaponGroups[GroupIdx].WeaponsInGroup.Num() - 1;  WeapIdx >=0 ; WeapIdx--)
				{
					AUTWeapon* CurrentWeapon = WeaponGroups[GroupIdx].WeaponsInGroup[WeapIdx];
					bool bSelected = CurrentWeapon == SelectedWeapon;

					if (bSelected)
					{
						bSelectedGroup = true;
					}
					Opacity = bSelected ? 1.0 : InactiveOpacity;

					// Draw the background and the background's border.
					int32 Idx = (WeapIdx == 0) ? 0 : 1;
					float FullIconCellWidth = (CurrentWeapon->Group == SelectedGroup) ? CellWidth * SelectedCellScale : CellWidth;
					float FullCellWidth = FullIconCellWidth + HeaderTab[Idx].GetWidth() + 3 + GroupHeaderCap[Idx].GetWidth();
					float CellScale = bSelected ? SelectedCellScale : 1.0;
					float CellHeight = CellBackground[Idx].GetHeight() * CellScale;
					float IconCellWidth = CellWidth * CellScale;
					float XPosition = (FullCellWidth * -1);
					YPosition -= HeaderTab[Idx].GetHeight() * CellScale;

					// Draw the Tab.
					RenderObj_TextureAt(HeaderTab[Idx], XPosition, YPosition, HeaderTab[Idx].GetWidth(), CellHeight);
					XPosition += HeaderTab[Idx].GetWidth();
					TextXPosition = XPosition;

					// Draw the Stretch bar

					// Calculate the size of the stretch bar.
					float StretchSize = FMath::Abs<float>(XPosition) -IconCellWidth - GroupHeaderCap[Idx].GetWidth();
					RenderObj_TextureAt(GroupSpacerBorder[Idx], XPosition, YPosition, StretchSize, CellHeight);
					XPosition += StretchSize;

					// Draw the cap
					RenderObj_TextureAt(GroupHeaderCap[Idx], XPosition, YPosition, GroupHeaderCap[Idx].GetWidth(), CellHeight);
					XPosition += GroupHeaderCap[Idx].GetWidth();

					// Draw the cell and the icon.
					RenderObj_TextureAt(CellBackground[Idx], XPosition, YPosition, IconCellWidth, CellHeight);
					RenderObj_TextureAt(CellBorders[Idx], XPosition, YPosition, IconCellWidth, CellHeight);

					Opacity = bSelected ? 1.0 : InactiveIconOpacity;

					// Draw the Weapon Icon
					if (CurrentWeapon)
					{
						WeaponIcon.UVs = bSelected ? CurrentWeapon->WeaponBarSelectedUVs : CurrentWeapon->WeaponBarInactiveUVs;
						WeaponIcon.RenderColor = UTHUDOwner->bUseWeaponColors ? CurrentWeapon->IconColor : FLinearColor::White;
					}

					float WeaponY = (CellHeight * 0.5) - (WeaponIcon.UVs.VL * CellScale * 0.5);
					RenderObj_TextureAt(WeaponIcon, -15, YPosition + WeaponY, WeaponIcon.UVs.UL * CellScale, WeaponIcon.UVs.VL * CellScale);

					// Draw the ammo bars
					if (BarTexture)
					{
						float AmmoPerc = CurrentWeapon->MaxAmmo > 0 ? float(CurrentWeapon->Ammo) / float(CurrentWeapon->MaxAmmo) : 0.0;
						float BarHeight = CellHeight - 16;
						float Width = bSelected ? 9.0 : 7.0;
						float X = (Width * -1) - 2;
						float Y = YPosition + 8;
						DrawTexture(BarTexture, X, Y, Width, BarHeight, BarTextureUVs.U, BarTextureUVs.V, BarTextureUVs.UL, BarTextureUVs.VL, 1.0, FLinearColor::Black);

						Y = Y + BarHeight - 1 - ((BarHeight-2) * AmmoPerc);
						BarHeight = (BarHeight -2) * AmmoPerc;
						FLinearColor BarColor = FLinearColor::Green;
						if (AmmoPerc <= 0.33)
						{
							BarColor = FLinearColor::Red;
						}
						else if (AmmoPerc <= 0.66)
						{
							BarColor = FLinearColor::Yellow;
						}
						DrawTexture(BarTexture, X+1, Y, Width-2, BarHeight, BarTextureUVs.U, BarTextureUVs.V, BarTextureUVs.UL, BarTextureUVs.VL, 1.0, BarColor);
					}
				}

				Opacity = bSelectedGroup ? 1.0 : InactiveIconOpacity;

				// @TODO FIXME - should show the actual key bound to the weapon group
				if (WeaponGroups[GroupIdx].Group == 0)
				{
					GroupText.Text = FText::FromString(TEXT("Q"));
				}
				else if (WeaponGroups[GroupIdx].Group == 10)
				{
					GroupText.Text = FText::FromString(TEXT("0"));
				}
				else
				{
					GroupText.Text = FText::AsNumber(WeaponGroups[GroupIdx].Group);
				}

				RenderObj_TextAt(GroupText, TextXPosition + GroupText.Position.X, YPosition + ((Y2 - YPosition) * 0.5) + GroupText.Position.Y);
			}
			else
			{
				Opacity = UTHUDOwner->HUDWidgetWeaponBarEmptyOpacity * InactiveOpacity;

				// Draw the background and the background's border.
				float FullIconCellWidth = CellWidth;
				float FullCellWidth = FullIconCellWidth + HeaderTab[0].GetWidth() + 3 + GroupHeaderCap[0].GetWidth();
				float CellScale = 1.0;
				float CellHeight = CellBackground[0].GetHeight() * CellScale;
				float IconCellWidth = CellWidth * CellScale;
				float XPosition = (FullCellWidth * -1);
				YPosition -= HeaderTab[0].GetHeight() * CellScale;

				// Draw the Tab.
				RenderObj_TextureAt(HeaderTab[0], XPosition, YPosition, HeaderTab[0].GetWidth(), CellHeight);
				XPosition += HeaderTab[0].GetWidth();
				TextXPosition = XPosition;

				// Draw the Stretch bar
				// Calculate the size of the stretch bar.
				float StretchSize = FMath::Abs<float>(XPosition) -IconCellWidth - GroupHeaderCap[0].GetWidth();
				RenderObj_TextureAt(GroupSpacerBorder[0], XPosition, YPosition, StretchSize, CellHeight);
				XPosition += StretchSize;

				// Draw the cap
				RenderObj_TextureAt(GroupHeaderCap[0], XPosition, YPosition, GroupHeaderCap[0].GetWidth(), CellHeight);
				XPosition += GroupHeaderCap[0].GetWidth();

				// Draw the cell and the icon.
				RenderObj_TextureAt(CellBackground[0], XPosition, YPosition, IconCellWidth, CellHeight);
				RenderObj_TextureAt(CellBorders[0], XPosition, YPosition, IconCellWidth, CellHeight);

				Opacity = UTHUDOwner->HUDWidgetWeaponBarEmptyOpacity * InactiveIconOpacity;
				GroupText.Text = FText::AsNumber(WeaponGroups[GroupIdx].Group);
				RenderObj_TextAt(GroupText, TextXPosition + GroupText.Position.X, YPosition + ((Y2 - YPosition) * 0.5) + GroupText.Position.Y);
			}

			YPosition -= 10;
		}
	}

	if (WeaponNameText.RenderOpacity > 0.0)
	{
		Opacity = 1.0;

		// Recalc this to handle aspect ratio
		WeaponNameText.Position.X = (Canvas->ClipX * 0.5) / RenderScale * -1;
		RenderObj_TextAt(WeaponNameText, WeaponNameText.Position.X, WeaponNameText.Position.Y);
	}

	if (WeaponNameDisplayTimer > 0)
	{
		WeaponNameDisplayTimer -= DeltaTime;
		if ( WeaponNameDisplayTimer <= (WeaponNameDisplayTime * 0.5f) )
		{
			WeaponNameText.RenderOpacity = WeaponNameDisplayTimer / (WeaponNameDisplayTime * 0.5f);
		}
		else
		{
			Opacity = 1.0;
		}
	}
}

void UUTHUDWidget_WeaponBar::CollectWeaponData(TArray<FWeaponGroup> &WeaponGroups, float DeltaTime)
{
	if (UTCharacterOwner)
	{
		// Parse over the character and see what weapons they have.
		if (RequiredGroups >= 0)
		{
			for (int i=RequiredGroups;i>=0;i--)
			{
				FWeaponGroup G = FWeaponGroup(i, NULL);
				WeaponGroups.Add(G);
			}
		}

		for (TInventoryIterator<AUTWeapon> It(UTCharacterOwner); It; ++It)
		{
			AUTWeapon* Weapon = *It;

			int32 GroupIndex = -1;
			for (int32 i=0;i<WeaponGroups.Num();i++)
			{
				if (WeaponGroups[i].Group == Weapon->Group)
				{
					GroupIndex = i;
					break;
				}
			}
	
			if (GroupIndex < 0)
			{
				FWeaponGroup G = FWeaponGroup(Weapon->Group, Weapon);

				int32 InsertPosition = -1;
				for (int32 i=0;i<WeaponGroups.Num();i++)
				{
					if ( WeaponGroups[i].Group < G.Group)
					{
						InsertPosition = i;
						break;
					}
				}

				if (InsertPosition <0)
				{
					WeaponGroups.Add(G);
				}
				else
				{
					WeaponGroups.Insert(G,InsertPosition);
				}
			}
			else
			{
				WeaponGroups[GroupIndex].WeaponsInGroup.Add(Weapon);
			}
		}
	}
}

float UUTHUDWidget_WeaponBar::GetDrawScaleOverride()
{
	return UTHUDOwner->HUDWidgetScaleOverride * UTHUDOwner->HUDWidgetWeaponBarScaleOverride;
}
