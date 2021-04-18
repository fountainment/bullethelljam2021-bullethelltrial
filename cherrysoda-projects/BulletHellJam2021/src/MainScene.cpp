#include "MainScene.h"

#include <CherrySoda/CherrySoda.h>

using namespace cherrysoda;
using main::MainScene;

static const BitTag s_bulletTag("bullet");
static const BitTag s_backgroundTag("background");
static const BitTag s_screenTextureTag("screen_texture");

static Pool<Circle, 1000> s_circlePool;
static Pool<Entity, 1005> s_bulletPool;

constexpr type::UInt16 kBackgroundPass = 1;
constexpr type::UInt16 kMainPass = 2;
constexpr type::UInt16 kScreenTexturePass = 3;

class BulletComponent : public Component
{
public:
	CHERRYSODA_DECLARE_COMPONENT(BulletComponent, Component);

	BulletComponent(const Math::Vec2& speed)
	: base(true, false)
	{
		m_speed = speed;
	}

	void Update() override
	{
		float deltaTime = Engine::Instance()->DeltaTime();
		GetEntity()->MovePosition2D(m_speed * deltaTime);
		if (m_speed != Vec2_Zero) {
		//	GetEntity()->Get<Sprite>()->ZRotation(Calc::Angle(m_speed));
		}
	}

private:
	Math::Vec2 m_speed;
};

static Pool<BulletComponent, 1005> s_bulletCompPool;

class CircleGraphicsComponent : public GraphicsComponent
{
public:
	CHERRYSODA_DECLARE_COMPONENT(CircleGraphicsComponent, GraphicsComponent);

	CircleGraphicsComponent(int radius)
	: base(false)
	, m_radius(radius)
	{}

	void Render() override
	{
		Draw::Circle(Math::Vec2(RenderPosition()), m_radius * 3.f, GetColor());
		Draw::Circle(Math::Vec2(RenderPosition()), m_radius, Color::DarkYellow);
		Draw::Circle(Math::Vec2(RenderPosition()), 1.f, Color::Yellow);
		Draw::Circle(Math::Vec2(RenderPosition()), 2.f, Color::Yellow);
	}

private:
	int m_radius;
};

class ProjectileComponent : public Component
{
public:
	CHERRYSODA_DECLARE_COMPONENT(ProjectileComponent, Component);

	ProjectileComponent() : base(true, false) {}

	static void InitSpriteBank()
	{
		ms_bulletAtlas = Atlas::FromAtlas("assets/atlases/atlas.json");
		ms_spriteBank = new SpriteBank(ms_bulletAtlas, "assets/sprites.json");
	}

	void Update() override
	{
		static float s_angle = 0.f;
		float deltaTime = Engine::Instance()->DeltaTime();
		float gameTime = Engine::Instance()->GameTime();
		if (!s_circlePool.IsFull()) {
			auto circle = s_circlePool.Create(4.f);
			auto entity = s_bulletPool.Create();
			entity->Position2D(Math::Vec2(90.f) + Calc::AngleToVector(s_angle, 120.f));
			entity->Add(ms_spriteBank->Create("blink"));
			entity->Add(s_bulletCompPool.Create(Calc::AngleToVector(-s_angle, 50.f + 5.f * gameTime)));
			entity->SetCollider(circle);
			entity->Tag(s_bulletTag);
			GetScene()->Add(entity);
			s_angle += deltaTime * 100.f;
		}
	}

private:
	static Atlas* ms_bulletAtlas;
	static SpriteBank* ms_spriteBank;
};

Atlas* ProjectileComponent::ms_bulletAtlas = nullptr;
SpriteBank* ProjectileComponent::ms_spriteBank = nullptr;

class ProgressGraphicsComponent : public GraphicsComponent
{
public:
	CHERRYSODA_DECLARE_COMPONENT(ProgressGraphicsComponent, GraphicsComponent);

	ProgressGraphicsComponent()
		: base(true)
	{}

	void Render() override
	{
		Draw::LineAngle(Math::Vec2(RenderPosition()), 0.f, 180.f * m_progress, Color::Yellow);
	}

	void Update() override
	{
		m_progress += Engine::Instance()->DeltaTime() / 10.f;
		if (m_progress > 1.f) {
			Engine::Instance()->TimeRate(0.f);
		}
	}

private:
	float m_progress = 0.f;
};

class PlayerControl : public Component
{
public:
	CHERRYSODA_DECLARE_COMPONENT(PlayerControl, Component);

	PlayerControl() : base(true, false) {}

	void Update() override
	{
		auto entity = GetEntity();
		float deltaTime = Engine::Instance()->DeltaTime();
		m_remainder += MInput::GamePads(0)->GetLeftStick(0.2f) * 100.f * deltaTime;
		auto move = Math_Round(m_remainder);
		m_remainder -= move;
		entity->MovePosition2D(move);
		if (entity->Left() < 0.f) entity->Left(0.f);
		if (entity->Right() > 180.f) entity->Right(180.f);
		if (entity->Bottom() < 0.f) entity->Bottom(0.f);
		if (entity->Top() > 180.f) entity->Top(180.f);
	}

private:
	Math::Vec2 m_remainder = Vec2_Zero;
};

class ScreenSpaceQuadGraphicsComponent : public GraphicsComponent
{
public:
	CHERRYSODA_DECLARE_COMPONENT(ScreenSpaceQuadGraphicsComponent, GraphicsComponent);

	ScreenSpaceQuadGraphicsComponent(const Texture2D& texture)
		: base(false)
		, m_texture(texture)
	{}

	void Render() override
	{
		Graphics::SetTexture(&m_texture);
		Graphics::ScreenSpaceQuad(m_texture.Width(), m_texture.Height(), Graphics::IsOriginBottomLeft());
		Graphics::SetStateNoDepth();
		Graphics::SubmitOnCurrentRenderPass();
	}

private:
	Texture2D m_texture;
};

void MainScene::Begin()
{
	Graphics::SetPointTextureSampling();

	ProjectileComponent::InitSpriteBank();

	Graphics::SetRenderPassOrder({ 0, kBackgroundPass, kMainPass, kScreenTexturePass });

	Graphics::UseRenderPass(kBackgroundPass)->SetClearColor(Color::Black);
	Graphics::UseRenderPass(kMainPass)->SetClearDiscard();
	Graphics::UseRenderPass(kScreenTexturePass)->SetClearDiscard();

	m_mainScreenTarget = new RenderTarget2D(180.0f, 180.0f);

	m_backgroundRenderer = new SingleTagRenderer(s_backgroundTag);
	m_mainRenderer = new TagExcludeRenderer(s_backgroundTag | s_screenTextureTag);
	m_screenTexRenderer = new SingleTagRenderer(s_screenTextureTag);

	m_backgroundRenderer->RenderPass(kBackgroundPass);
	m_mainRenderer->RenderPass(kMainPass);
	m_screenTexRenderer->RenderPass(kScreenTexturePass);

	m_backgroundRenderer->SetRenderTarget(m_mainScreenTarget);
	m_mainRenderer->SetRenderTarget(m_mainScreenTarget);

	m_backgroundRenderer->SetEffect(Effect::LoadEffectFromFile("background"));
	m_mainRenderer->SetEffect(Graphics::GetEmbeddedEffect("sprite"));
	m_screenTexRenderer->SetEffect(Effect::LoadEffectFromFile("screenspacequad"));

	m_backgroundRenderer->GetCamera()->Position(Math::Vec3(0.f, 0.f, 1.f));
	m_backgroundRenderer->GetCamera()->UseOrthoProjection(true);

	m_mainRenderer->GetCamera()->Position(Math::Vec3(0.f, 0.f, 1.f));
	m_mainRenderer->GetCamera()->UseOrthoProjection(true);

	m_screenTexRenderer->GetCamera()->Position(Math::Vec3(m_mainScreenTarget->Width() * 0.5f, m_mainScreenTarget->Height() * 0.5f, 1.f));
	m_screenTexRenderer->GetCamera()->UseOrthoProjection(true);
	m_screenTexRenderer->GetCamera()->CenterOrigin();
	m_screenTexRenderer->GetCamera()->Scale2D(Math::Vec2(3.f));
	m_screenTexRenderer->KeepCameraCenterOrigin(true);

	Add(m_backgroundRenderer);
	Add(m_mainRenderer);
	Add(m_screenTexRenderer);

	auto backgroundEntity = new Entity();
	backgroundEntity->Add(new ScreenSpaceQuadGraphicsComponent(Texture2D::ForColorBuffer(180.f, 180.f)));
	backgroundEntity->Tag(s_backgroundTag);
	Add(backgroundEntity);

	m_player = new Entity();
	m_player->Add(new ProjectileComponent);
	m_player->Position2D(Math::Vec2(90.f, 90.f));
	m_player->Add(new CircleGraphicsComponent(3));
	m_player->Add(new PlayerControl);
	m_player->SetCollider(s_circlePool.Create(1));
	Add(m_player);

	auto progressBar = new Entity();
	progressBar->Depth(-1.f);
	progressBar->PositionY(179.f);
	progressBar->Add(new ProgressGraphicsComponent);
	Add(progressBar);

	auto screenTex = new Entity();
	auto screenSpaceQuad = new ScreenSpaceQuadGraphicsComponent(m_mainScreenTarget->GetTexture2D());
	screenTex->Tag(s_screenTextureTag);
	screenTex->Add(screenSpaceQuad);
	Add(screenTex);

	base::Begin();
}

void MainScene::Update()
{
	auto viewSize = Engine::Instance()->GetViewSize();
	int cameraScale = Math_Min(Math_Max(1.f, viewSize.x / (float)m_mainScreenTarget->Width()), \
		Math_Max(1.f, viewSize.y / (float)m_mainScreenTarget->Height()));
	m_screenTexRenderer->GetCamera()->Scale2D(Math::Vec2(cameraScale));

	static bool s_dead = false;
	auto bullet = m_player->CollideFirst(s_bulletTag);
	if (!s_dead && bullet) {
		Engine::Instance()->TimeRate(0.f);
		MInput::GamePads(0)->Rumble(100.f, 1.f);
		s_dead = true;
	}
	if (s_dead) {
		static float s_deadTime = 0.f;
		if (s_deadTime > 0.5f) {
			MInput::GamePads(0)->StopRumble();
		}
		s_deadTime += Engine::Instance()->RawDeltaTime();
	}

	base::Update();
}
