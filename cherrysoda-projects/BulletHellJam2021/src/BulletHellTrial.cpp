#include "BulletHellTrial.h"

#include "MainScene.h"

#include <CherrySoda/CherrySoda.h>

using bullethelltrial::BulletHellTrial;

using namespace cherrysoda;

SpriteBank* BulletHellTrial::ms_spriteBank = nullptr;

BulletHellTrial::BulletHellTrial()
	: base(720, 720, "BulletHellTrial")
{
	SetClearColor(Color::Black);
}

void BulletHellTrial::Initialize()
{
	base::Initialize();

	Audio::MasterVolume(1.0);
	Graphics::SetPointTextureSampling();
	Graphics::CreateUniformVec4("u_data");

#ifndef CHERRYSODA_ENABLE_DEBUG
	GUI::Disable();
#endif

	auto scene = new main::MainScene();
	SetScene(scene);
}

void BulletHellTrial::LoadContent()
{
	base::LoadContent();

	ms_spriteBank = new SpriteBank("assets/atlases/atlas.json", "assets/sprites.json");
}

void BulletHellTrial::UnloadContent()
{
	delete ms_spriteBank;
	ms_spriteBank = nullptr;

	base::UnloadContent();
}
