#include "BulletHellTrial.h"

#include "MainScene.h"

#include <CherrySoda/CherrySoda.h>

using bullethelltrial::BulletHellTrial;

using namespace cherrysoda;

BulletHellTrial::BulletHellTrial()
	: base(720, 720, "BulletHellTrial")
{
	SetClearColor(Color::Black);
}

void BulletHellTrial::Update()
{
	base::Update();

	// Add global GUI or other global stuffs here
}

void BulletHellTrial::Initialize()
{
	base::Initialize();

	Audio::MasterVolume(1.0);

	// Initialize and set scene here
	auto scene = new main::MainScene();
	SetScene(scene);
}

void BulletHellTrial::LoadContent()
{
	base::LoadContent();

	// Load textures, sprites, shaders and other resources here
}
