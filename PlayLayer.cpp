#include "PlayLayer.h"
#include "GameManager.h"
#include "MinHook.h"
#include <iomanip>

bool PlayLayer::wouldDie = false;
int PlayLayer::frames = 0;
int PlayLayer::deaths = 0;
float PlayLayer::totalDelta = 0;
float PlayLayer::prevX = 0;

std::string getAccuracyText() {
	if (PlayLayer::frames == 0) return "Accuracy: 100.00%";
	float p = (float)(PlayLayer::frames - PlayLayer::deaths) / (float)PlayLayer::frames;
	std::stringstream stream;
	stream << "Accuracy: " << std::fixed << std::setprecision(2) << p * 100.f << "%";
	return stream.str();
}

bool __fastcall PlayLayer::initHook(CCLayer* self, int edx, void* GJGameLevel) {
	size_t base = (size_t)GetModuleHandle(0);

	prevX = 0;

	if (GameManager::getGameVariable(*(void**)(base + 0x3222d0), "noclip")) {
		CCLabelBMFont* textObj = CCLabelBMFont::create("Accuracy: 100.00%", "goldFont.fnt");

		textObj->setZOrder(1000);
		textObj->setTag(100000);

		textObj->setScale(0.5);
		auto size = textObj->getScaledContentSize();

		textObj->setPosition({ size.width / 2 + 3, size.height / 2 + 3});

		self->addChild(textObj);
	}
	return init(self, GJGameLevel);
}

void __fastcall PlayLayer::hkUpdate(cocos2d::CCLayer* self, void* edx, float delta) {
	void* player1 = *(void**)((char*)self + 0x224);
	float x = *(float*)((size_t)player1 + 0x67c);

	if (x != prevX) {
		frames += 1;
		totalDelta += delta;
	}

	size_t base = (size_t)GetModuleHandle(0);

	if (GameManager::getGameVariable(*(void**)(base + 0x3222d0), "noclip")) {
		CCLabelBMFont* textObj = (CCLabelBMFont*)self->getChildByTag(100000);
		auto text = getAccuracyText();
		textObj->setString(text.c_str());
		auto size = textObj->getScaledContentSize();

		textObj->setPosition({ size.width / 2 + 3, size.height / 2 + 3});
	}


	if (wouldDie) {
		wouldDie = false;
		if (totalDelta >= 0.1) {
			deaths += 1;
		}
	}

	prevX = x;
	return update(self, delta);
}

int __fastcall PlayLayer::hkResetLevel(void* self) {
	int res = resetLevel(self);
	void* player1 = *(void**)((char*)self + 0x224);
	prevX = *(float*)((size_t)player1 + 0x67c);
	frames = 0;
	totalDelta = 0;
	deaths = 0;
	wouldDie = false;

	return res;
}

int __fastcall PlayLayer::hkDeath(void* self, void*, void* go, void* powerrangers) {
	size_t base = (size_t)GetModuleHandle(0);

	if (GameManager::getGameVariable(*(void**)(base + 0x3222d0), "noclip")) {

		uintptr_t address = base + 0x20A8Ba;

		wouldDie = true;
		__asm {
			mov eax, address
			jmp eax
		}
	}
	else {
		return death(self, go, powerrangers);
	}
	
}

void PlayLayer::mem_init() {
	size_t base = (size_t)GetModuleHandle(0);

	MH_CreateHook(
		(PVOID)(base + 0x01FB780),
		PlayLayer::initHook,
		(LPVOID*)&PlayLayer::init);

	MH_CreateHook(
		(PVOID)(base + 0x20A1A0),
		PlayLayer::hkDeath,
		(LPVOID*)&PlayLayer::death);

	MH_CreateHook(
		(PVOID)(base + 0x20BF00),
		PlayLayer::hkResetLevel,
		(LPVOID*)&PlayLayer::resetLevel);

	MH_CreateHook(
		(PVOID)(base + 0x2029C0),
		PlayLayer::hkUpdate,
		(LPVOID*)&PlayLayer::update);
}