#pragma once

/*
	gon_iss (c) 2025
*/

#include <game_sa/CSprite2d.h>

#include "plugin.h"

#include "../../RadioSystem.h"

#include "../../Sound/Radio/RadioStation.h"
#include "../../Sound/InterfaceSounds.h"
#include "../../Visual/RadioIcons.h"
#include "../../Utils/Utils.h"
#include "../../Utils/Mouse.h"
#include "../../Utils/Timer.h"
#include "../../Utils/TimeDistorter.h"
#include "../../Utils/HudController.h"

#include "RadioWheelUtils.h"
#include "RadioWheelFactories.h"
#include "WheelGeometryCalculator.h"

using namespace plugin;

class RadioWheel
{

protected:

	TimeDistortion prevTimeDistortion = TimeDistortion::Normal;
	TimeDistortion currentTimeDistortion = TimeDistortion::Normal;

	bool init = false;

	void HandleShowing(int CurrentRadioId) {

		UpdateRadioInfo(currentRadioId_);
		SetSelectedStation(CurrentRadioId);

		state_.showedInitialStation = false;
		state_.initMouseLock = true;

		Utils::DisableCameraMovement();

		//TimeDistorter::SetTarget(TimeDistortion::Slowed);
		// now it's in the CheckTimeDistortion

		Mouse::CenterMousePosition();
		ResoundAudioEngine::ActivateSlowMoMode(SlowMoType::AUD_SLOWMO_RADIOWHEEL);
		
		Update(SCREEN_COORD(Mouse::currentMousePosition.x), SCREEN_COORD(Mouse::currentMousePosition.y));

		CheckHudShown();
	}

	void CheckHudShown() {
		HudController::SetHudShown(state_.isShowing);
	}

	void HandleHiding() {

		RadioInfoVisual::HideForcely();
		Utils::ResetCameraMovement();

		ResoundAudioEngine::DeActivateSlowMoMode(SlowMoType::AUD_SLOWMO_RADIOWHEEL);

		if (tmrTuneRadio_.IsStarted()) {
			ProcessSelectionConfirm(items_[state_.selectedIndex]);
		}

		CheckHudShown();
	}

	std::unique_ptr<IRadioWheelItemFactory> factory_;
	std::vector<RadioWheelItem> items_;

	int currentRadioId_ = 1;
	int lastKnownRadioId = -1;

	struct Config {
		float minRadiusScreenHeightRelation = 2.942779f;
		float defaultRadius = 450.0f;
		float minIconSize = 75.0f;
		float maxIconSize = 97.0f;
		int retuneTime = 500;
	};

	Config config_;
	Timer<CurrentTime> tmrTuneRadio_;

public:

	enum class WheelType {
		Standard, Custom, Combined
	};

	struct State {
		int selectedIndex = 1;
		int prevSelectedIndex = 1;
		int initialIndex = 1;
		float lastMouseAngle = 270.0f;

		float initMouseX = 0;
		float initMouseY = 0;

		float currentRadius = 0;
		float currentIconSize = 0;
		bool needGeometryUpdate = true;

		WheelGeometry geometry;

		//std::vector<WheelPositionCalculator::Position> positions; //is it actually needed in the future or what

		bool showedInitialStation = false;
		bool initMouseLock = true;
		bool isShowing = false;
		bool isSlowed = false;
		WheelType currentType = WheelType::Custom;
	};

	State state_;

	using SelectionHandler = std::function<void(const RadioWheelItem& item)>;

	SelectionHandler OnSelectionChanged;
	SelectionHandler OnSelectionSelected;

	RadioStation* selectedStation = nullptr;

	/*getRadioOffIcon_(getRadioOffIcon),
		getSelectorIcon_(getSelectorIcon),*/
		//std::function<CSprite2d* ()> getRadioOffIcon,
	//std::function<CSprite2d* ()> getSelectorIcon,
	// i had an idea to use the callbacks for the flexible structure


	RadioWheel(
		Config config = {}
	) : 
		config_(config),
		state_{} {}

	void Initialize(std::unique_ptr<IRadioWheelItemFactory> factory) {

		if (!init) {
			init = true;
		}
		else { return; }

		factory_ = std::move(factory);
		items_ = factory_->createItems();

		Events::drawingEvent += [this ] {

			Mouse::UpdateMousePosition();
			Update(SCREEN_COORD(Mouse::currentMousePosition.x), SCREEN_COORD(Mouse::currentMousePosition.y));
			Render();
			CheckTimeDistortion(state_);

		};
	}

	void SyncWithSystem(int currentId) {
		SetSelectedStation(currentId);
		lastKnownRadioId = currentId;
	}

	void SetSelectedStation(int stationId) {
		for (int i = 0; i < items_.size(); ++i) {
			if (items_[i].stationId == stationId) {
				if (state_.selectedIndex != i) {
					state_.prevSelectedIndex = state_.selectedIndex;
					state_.selectedIndex = i;

					// instant UI update
					if (state_.isShowing) {
						ProcessSelectionChange(items_[i]);
					}
				}
				return;
			}
		}
		// If the station was never found - set 'Radio off'
		for (int i = 0; i < items_.size(); ++i) {
			if (items_[i].type == RadioWheelItem::Type::Off) {
				state_.selectedIndex = i;
				break;
			}
		}

		currentRadioId_ = stationId;
	}

	void UpdateSelection(float mouseX, float mouseY) {
		const float centerX = RsGlobal.maximumWidth / 2;
		const float centerY = RsGlobal.maximumHeight / 2;

		state_.selectedIndex = GetSelectedSectorCircular(
			mouseX, mouseY,
			centerX, centerY,
			state_.geometry.radius,
			items_.size(),
			state_.lastMouseAngle,
			state_.selectedIndex
		);

		if (state_.selectedIndex != state_.prevSelectedIndex) {
			ProcessSelectionChange(items_[state_.selectedIndex]);
			state_.prevSelectedIndex = state_.selectedIndex;
		}

		if (tmrTuneRadio_.IsStarted() && tmrTuneRadio_.IsComplete(config_.retuneTime)) {
			ProcessSelectionConfirm(items_[state_.selectedIndex]);
		}
	}

	void SetWheelType(WheelType newType, std::vector<RadioStation*> customStations = {}) {
		state_.currentType = newType;
		updateFactory(newType, customStations);
		items_ = factory_->createItems();
		resetSelectionState();
	}

	void CheckTimeDistortion(State state_) {

		if (!state_.isShowing) {
			state_.isSlowed = false;
			TimeDistorter::SetTargetTimeDistort(TimeDistortion::Normal);
		}

		if (state_.isSlowed) {
			TimeDistorter::SetTargetTimeDistort(TimeDistortion::Slowed);
		}
	}

	void Show(int CurrentRadioId) {
		if (!state_.isShowing) {
			HandleShowing(CurrentRadioId);
		}

		state_.isSlowed = true;
		state_.isShowing = true;
	}

	void Hide() {
		if (state_.isShowing) {
			HandleHiding();
		}
		state_.isSlowed = false;
		state_.isShowing = false;
	}

	void Render() {

		if (!state_.isShowing) return;

		RadioInfoVisual::DrawRadioInfo(Fonts::fArtist, Fonts::fTrack, true);

		const float centerX = RsGlobal.maximumWidth / 2;
		const float centerY = RsGlobal.maximumHeight / 2;
		const float radius = state_.geometry.radius;
		const float iconSize = state_.geometry.iconSize;
		const int itemsCount = items_.size();

		const float angleStep = 360.0f / itemsCount;
		float angle = 270.0f; // Start angle

		angle = angle + angleStep;

		for (int i = 0; i < itemsCount; ++i) {
			const auto& item = items_[i];
			const bool isSelected = (i == state_.selectedIndex);
			const int opacity = isSelected ? 255 : 125;

			// Calculate the position
			float iconX, iconY;
			if (item.type == RadioWheelItem::Type::Off) {
				// "Radio Off" always at the bottom
				iconX = centerX;
				iconY = centerY + radius;
			}
			else {
				const float angleRad = angle * (M_PI / 180.0f);
				iconX = centerX + radius * std::cos(angleRad);
				iconY = centerY - radius * std::sin(angleRad);
				angle += angleStep;
			}

			// Icon
			if (item.icon && item.icon->m_pTexture) {
				item.icon->Draw(
					iconX - iconSize / 2,
					iconY - iconSize / 2,
					iconSize, iconSize,
					CRGBA(255, 255, 255, opacity)
				);
			}

			// Selector
			if (isSelected && RadioIcons::radio_selector) {
				const float selectorSize = iconSize + 10;
				RadioIcons::radio_selector->Draw(
					iconX - selectorSize / 2,
					iconY - selectorSize / 2,
					selectorSize, selectorSize,
					CRGBA(255, 133, 85, 255)
				);
			}
		}
	}


	void Update(float mouseX, float mouseY) {
		if (!state_.isShowing) return;

		// Recalculate geometry only on change
		if (state_.needGeometryUpdate) {
			const float screenWidth = RsGlobal.maximumWidth;
			const float screenHeight = RsGlobal.maximumHeight;

			state_.geometry = WheelGeometryCalculator::Calculate(
				items_.size(),
				screenWidth,
				screenHeight,
				config_.minIconSize,
				config_.maxIconSize
			);

			state_.needGeometryUpdate = false;
		}

		UpdateSelection(mouseX, mouseY);
	}

	int GetIconsCount(WheelType currentType, int customStationsSize) {
		switch (currentType) {
		case WheelType::Standard: return RadioIcons::icons_round.size();
		case WheelType::Custom: return customStationsSize + 1;
		case WheelType::Combined: return RadioIcons::icons_round.size() + customStationsSize;
		default: return 0;
		}
	}

	protected:

		void ProcessSelectionChange(const RadioWheelItem& newSelection) {
			tmrTuneRadio_.Start();
			state_.showedInitialStation = true;

			if (OnSelectionChanged) {
				OnSelectionChanged(newSelection);
			}

			UpdateRadioInfo(newSelection);
		}

		void UpdateRadioInfo(int selectedIndex) {

			if (selectedIndex >= 0 && selectedIndex < items_.size()) {

				const auto& selected = items_[state_.selectedIndex];
				UpdateRadioInfo(selected);
			}
		}

		void UpdateRadioInfo(const RadioWheelItem& wheelItem) {

			auto& station = wheelItem.customStation;

			if (station) {
				RadioInfoVisual::SetInfo(
					station->name,
					station->TryGetArtistTitle().first,
					station->TryGetArtistTitle().second,
					station->name,
					true
				);
			}
			else {
				RadioInfoVisual::SetInfo(
					wheelItem.name,
					wheelItem.artist,
					wheelItem.title,
					wheelItem.name,
					true
				);
			}
		}

		void ProcessSelectionConfirm(const RadioWheelItem& selection) {
			tmrTuneRadio_.Zero();

			if (OnSelectionSelected) {
				OnSelectionSelected(selection);
			}
		}

		void updateFactory(RadioWheel::WheelType type, std::vector<RadioStation*> customStations) {
			switch (type) {
			case WheelType::Standard:
				factory_ = std::make_unique<StandardWheelFactory>();
				break;
			case WheelType::Custom:
				factory_ = std::make_unique<CustomWheelFactory>(customStations);
				break;
			case WheelType::Combined:
				factory_ = std::make_unique<CombinedWheelFactory>(customStations);
				break;
			}
		}

		void resetSelectionState() {
			state_.selectedIndex = 0;
			state_.prevSelectedIndex = -1;
			state_.showedInitialStation = false;
			// ...
		}
};