#include "MainScene.h"

#include <CherrySoda/CherrySoda.h>

using namespace cherrysoda;
using main::MainScene;

static const BitTag s_bulletTag("bullet");
static const BitTag s_backgroundTag("background");
static const BitTag s_screenTextureTag("screen_texture");

static Pool<Circle, 1000> s_circlePool;
static Pool<Entity, 1100> s_bulletPool;
static Pool<Sprite, 1100> s_spritePool;

static int s_level = 0;

constexpr type::UInt16 kBackgroundPass = 1;
constexpr type::UInt16 kMainPass = 2;
constexpr type::UInt16 kScreenTexturePass = 3;

static float s_startTime = 0.f;

class BulletComponent : public Component
{
public:
	CHERRYSODA_DECLARE_COMPONENT(BulletComponent, Component);

	BulletComponent(const Math::Vec2& speed, bool rotationAlignWithSpeed = false)
	: base(true, false)
	{
		m_speed = speed;
		m_rotationAlignWithSpeed = rotationAlignWithSpeed;
	}

	void Update() override
	{
		float deltaTime = Engine::Instance()->DeltaTime();
		GetEntity()->MovePosition2D(m_speed * deltaTime);
		if (m_rotationAlignWithSpeed && m_speed != Vec2_Zero) {
			GetEntity()->Get<Sprite>()->ZRotation(Calc::Angle(m_speed));
		}
	}

private:
	Math::Vec2 m_speed;
	bool m_rotationAlignWithSpeed;
};

static Pool<BulletComponent, 1100> s_bulletCompPool;

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

	static void DestroySpriteBank()
	{
		delete ms_spriteBank;
		delete ms_bulletAtlas;
	}

	void Update() override
	{
		static float s_angle = 0.f;
		float deltaTime = Engine::Instance()->DeltaTime();
		float gameTime = Engine::Instance()->GameTime() - s_startTime;
		switch (s_level) {
		case 0:
			if (GetScene()->OnInterval(1.f / 60.f)) {
				if (!s_circlePool.IsFull()) {
					auto entity = s_bulletPool.Create();
					auto circle = s_circlePool.Create(4.f);
					auto sprite = ms_spriteBank->CreateOn(s_spritePool.Create(), "blink");
					auto bulletComp = s_bulletCompPool.Create(Calc::AngleToVector(-s_angle, 50.f + 5.f * gameTime));
					entity->Position2D(Math::Vec2(90.f) + Calc::AngleToVector(s_angle, 120.f));
					entity->Add(sprite);
					entity->Add(bulletComp);
					entity->SetCollider(circle);
					entity->Tag(s_bulletTag);
					entity->OnRemoved(
						[circle, sprite, bulletComp](Entity* entity, Scene* scene)
						{
							s_circlePool.Destroy(circle);
							s_spritePool.Destroy(sprite);
							s_bulletCompPool.Destroy(bulletComp);
							s_bulletPool.Destroy(entity);
						});
					GetScene()->Add(entity);
					s_angle += deltaTime * 100.f;
				}
			}
			break;
		case 1:
			if (GetScene()->OnInterval(1.5f)) {
				if (!s_circlePool.IsFull()) {
					int t1 = Calc::GetRandom()->NextInt(30);
					int td = Calc::GetRandom()->NextInt(3, 8);
					int t = Calc::GetRandom()->Next(2);
					if (t) td *= -1;
					int t2 = (t1 + 15 + td) % 30;
					for (int i = 0; i < 30; ++i) {
						if (s_circlePool.IsFull()) {
							break;
						}
						if (abs((i + 45 - t1) % 30 - 15) < 2) continue;
						if (abs((i + 45 - t2) % 30 - 15) < 2) continue;
						float a = Math::Pi2 / 30.f;
						auto entity = s_bulletPool.Create();
						auto circle = s_circlePool.Create(4.f);
						auto bulletComp = s_bulletCompPool.Create(Calc::AngleToVector(a * i + Math::Pi, 100.f), true);
						auto sprite = ms_spriteBank->CreateOn(s_spritePool.Create(), "missile");
						entity->Position2D(Math::Vec2(90.f) + Calc::AngleToVector(a * i, 120.f));
						entity->SetCollider(circle);
						entity->Add(bulletComp);
						entity->Add(sprite);
						entity->Tag(s_bulletTag);
						entity->OnRemoved(
							[circle, sprite, bulletComp](Entity* entity, Scene* scene)
							{
								s_circlePool.Destroy(circle);
								s_spritePool.Destroy(sprite);
								s_bulletCompPool.Destroy(bulletComp);
								s_bulletPool.Destroy(entity);
							});
						GetScene()->Add(entity);
					}
				}
			}
			break;
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
			m_progress = 0.f;
			s_level++;
		}
	}

	void ResetProgress()
	{
		m_progress = 0.f;
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
		float speed = 120.f;
		if (MInput::GamePads(0)->Check(Buttons::RightShoulder)) speed = 70.f;
		Math::Vec2 leftStick = MInput::GamePads(0)->GetLeftStick(0.1f);
		Math::Vec2 keyboardAxis = MInput::Keyboard()->GetAxis(Keys::A, Keys::D, Keys::S, Keys::W);
		Math::Vec2 direction = Calc::SafeNormalize(leftStick) + Calc::SafeNormalize(keyboardAxis);
		direction = Calc::SafeNormalize(direction);
		m_remainder += direction * speed * deltaTime;
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

	m_progressBar = new Entity();
	m_progressBar->Depth(-1.f);
	m_progressBar->PositionY(179.f);
	m_progressBar->Add(new ProgressGraphicsComponent);
	Add(m_progressBar);

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

		if (MInput::GamePads(0)->Pressed(Buttons::A)) {
			s_bulletPool.Traverse([](auto bullet) { bullet->RemoveSelf(); });
			s_dead = false;
			s_deadTime = 0.f;
			s_startTime = Engine::Instance()->GameTime();
			Engine::Instance()->TimeRate(1.f);
			m_progressBar->Get<ProgressGraphicsComponent>()->ResetProgress();
		}
	}

	base::Update();
}
