#include "MainScene.h"

#include <CherrySoda/CherrySoda.h>

using namespace cherrysoda;
using main::MainScene;

static const BitTag s_bulletTag("bullet");
static const BitTag s_backgroundTag("background");
static const BitTag s_screenTextureTag("screen_texture");

static SpriteBank* s_spriteBank = nullptr;

static Pool<Circle, 1000> s_circlePool;
static Pool<Entity, 1100> s_bulletPool;
static Pool<Sprite, 1100> s_spritePool;

static int s_level = -1;

constexpr type::UInt16 kBackgroundPass = 1;
constexpr type::UInt16 kMainPass = 2;
constexpr type::UInt16 kScreenTexturePass = 3;

static bool s_dead = false;
static float s_deadTime = 0.f;
static float s_startTime = 0.f;
static float s_localTimeRate = 1.f;

static Audio::EventInstance s_bgm;

class BulletComponent : public Component
{
public:
	CHERRYSODA_DECLARE_COMPONENT(BulletComponent, Component);

	BulletComponent(const Math::Vec2& velocity, bool rotationAlignWithVelocity = false)
	: base(true, false)
	{
		m_velocity = velocity;
		m_rotationAlignWithVelocity = rotationAlignWithVelocity;
	}

	void Update() override
	{
		float deltaTime = Engine::Instance()->DeltaTime() * s_localTimeRate;
		auto entity = GetEntity();
		entity->MovePosition2D(m_velocity * deltaTime);
		if (m_rotationAlignWithVelocity && m_velocity != Vec2_Zero) {
			entity->Get<Sprite>()->ZRotation(Calc::Angle(m_velocity));
		}
		if (Math_LengthSq(entity->Position2D() - Math::Vec2(90.f)) > 150.f * 150.f) {
			entity->RemoveSelf();
		}
	}

	void Stop()
	{
		m_velocity = Vec2_Zero;
	}

private:
	Math::Vec2 m_velocity;
	bool m_rotationAlignWithVelocity;
};

static Pool<BulletComponent, 1100> s_bulletCompPool;

class CircleGraphicsComponent : public GraphicsComponent
{
public:
	CHERRYSODA_DECLARE_COMPONENT(CircleGraphicsComponent, GraphicsComponent);

	CircleGraphicsComponent(int radius)
	: base(false)
	, m_radius(radius)
	{
		SetColor(Color::Yellow);
	}

	void Render() override
	{
		Draw::Circle(Math::Vec2(RenderPosition()), m_radius * 3.f, Color::White);
		Draw::Circle(Math::Vec2(RenderPosition()), m_radius, Color::DarkYellow);
		Draw::Circle(Math::Vec2(RenderPosition()), 1.f, GetColor());
		Draw::Circle(Math::Vec2(RenderPosition()), 2.f, GetColor());
	}

private:
	int m_radius;
};

static Entity* CreateBullet(const Math::Vec2& position, const Math::Vec2& velocity, const StringID& id, bool rotationAlignWithVelocity = false)
{
	if (s_circlePool.IsFull()) {
		return nullptr;
	}
	auto entity = s_bulletPool.Create();
	auto circle = s_circlePool.Create(4.f);
	auto sprite = s_spriteBank->CreateOn(s_spritePool.Create(), id);
	auto bulletComp = s_bulletCompPool.Create(velocity, rotationAlignWithVelocity);
	entity->Position2D(position);
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
	return entity;
}

static void InitSpriteBank()
{
	s_spriteBank = new SpriteBank("assets/atlases/atlas.json", "assets/sprites.json");
}

// static void DestroySpriteBank()
// {
// 	delete s_spriteBank;
// }

class ProjectileComponent : public Component
{
public:
	CHERRYSODA_DECLARE_COMPONENT(ProjectileComponent, Component);

	ProjectileComponent() : base(true, false) {}

	void Update() override
	{
		static float s_angle = 0.f;
		static int s_count = 0;
		float gameTime = Engine::Instance()->GameTime() - s_startTime;
		switch (s_level) {
		case 0:
			if (GetScene()->OnInterval(1.f / 60.f)) {
				auto position = Math::Vec2(90.f) + Calc::AngleToVector(s_angle, 140.f);
				auto velocity = Calc::AngleToVector(-s_angle, 50.f + 5.f * gameTime);
				auto bullet = CreateBullet(position, velocity, "blink");
				if (bullet != nullptr) GetScene()->Add(bullet);
				s_angle += ((1.f / 60.f) + Calc::GetRandom()->NextFloat(-0.0001f, 0.0001f)) * 100.f;
			}
			break;
		case 1:
			if (GetScene()->OnInterval(1.5f)) {
				int t1 = Calc::GetRandom()->NextInt(30);
				int td = Calc::GetRandom()->NextInt(3, 8);
				int t = Calc::GetRandom()->Next(2);
				if (t) td *= -1;
				int t2 = (t1 + 15 + td) % 30;
				for (int i = 0; i < 30; ++i) {
					if (abs((i + 45 - t1) % 30 - 15) < 2) continue;
					if (abs((i + 45 - t2) % 30 - 15) < 2) continue;
					float a = Math::Pi2 / 30.f;
					auto position = Math::Vec2(90.f) + Calc::AngleToVector(a * i, 140.f);
					auto velocity = Calc::AngleToVector(a * i + Math::Pi, 100.f);
					auto bullet = CreateBullet(position, velocity, "missile", true);
					if (bullet == nullptr) break;
					GetScene()->Add(bullet);
				}
			}
			break;
		case 2:
			if (GetScene()->OnInterval(1.f / 15.f)) {
				float a = Math::Pi2 / 60.f;
				{
					auto position = Math::Vec2(90.f) + Calc::AngleToVector(a * s_count, 140.f);
					auto velocity = Calc::AngleToVector(a * s_count + Math::Pi, 100.f);
					auto bullet = CreateBullet(position, velocity, "breath");
					if (bullet != nullptr) GetScene()->Add(bullet);
				}
				{
					auto position = Math::Vec2(90.f) + Calc::AngleToVector(a * s_count + Math::Pi, 140.f);
					auto velocity = Calc::AngleToVector(a * s_count + Math::Pi2, 100.f);
					auto bullet = CreateBullet(position, velocity, "breath");
					if (bullet != nullptr) GetScene()->Add(bullet);
				}
				{
					auto position = Math::Vec2(90.f) + Calc::AngleToVector(a * s_count + Math::PiHalf, 140.f);
					auto velocity = Calc::AngleToVector(a * s_count + Math::PiHalf * 3.f, 100.f);
					auto bullet = CreateBullet(position, velocity, "breath");
					if (bullet != nullptr) GetScene()->Add(bullet);
				}
				{
					auto position = Math::Vec2(90.f) + Calc::AngleToVector(a * s_count + Math::PiHalf * 3.f, 140.f);
					auto velocity = Calc::AngleToVector(a * s_count + Math::PiHalf, 100.f);
					auto bullet = CreateBullet(position, velocity, "breath");
					if (bullet != nullptr) GetScene()->Add(bullet);
				}
				{
					auto position = Math::Vec2(90.f) + Calc::AngleToVector(a * s_count + Math::Pi, 140.f);
					auto velocity = Calc::AngleToVector(a * s_count + Math::Pi2, 100.f);
					auto bullet = CreateBullet(position, velocity, "breath");
					if (bullet != nullptr) GetScene()->Add(bullet);
				}
				s_count++;
			}
			break;
		case 3:
			{
				static bool firstTimeHere = true;
				if (firstTimeHere) {
					auto coverTween = GetEntity()->GetSceneAs<MainScene>()->m_coverTween;
					auto cover = GetEntity()->GetSceneAs<MainScene>()->m_cover;
					coverTween->OnUpdate([cover](Tween* tween) { cover->PositionY(tween->Eased() * 180.f); });
					coverTween->OnComplete([](Tween* tween) {});
					coverTween->Start(true);
					firstTimeHere = false;
				}
			}
			break;
		}
	}

private:
	static SpriteBank* ms_spriteBank;
};


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
		if (s_level >= 0 && s_level < 3) {
			m_progress += Engine::Instance()->DeltaTime() * s_localTimeRate / 10.f;
		}
		if (m_progress > 1.f) {
			m_progress = 0.f;
			s_bulletPool.Traverse(
				[](Entity* bullet)
				{
					bullet->Collidable(false);
					bullet->Get<BulletComponent>()->Stop();
					bullet->Get<Sprite>()->OnFinish([bullet](auto id) { bullet->RemoveSelf(); });
					bullet->Get<Sprite>()->Play("boom");
					bullet->Get<Sprite>()->ZRotation(Calc::GetRandom()->NextAngle());
				});
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
		float deltaTime = Engine::Instance()->DeltaTime() * s_localTimeRate;
		float speed = 120.f;
		if (MInput::GamePads(0)->Check(Buttons::RightShoulder)) speed = 70.f;
		if (MInput::Keyboard()->Check(Keys::LeftShift, Keys::RightShift)) speed = 70.f;
		Math::Vec2 leftStick = MInput::GamePads(0)->GetLeftStick(0.1f);
		Math::Vec2 keyboardAxis1 = MInput::Keyboard()->GetAxis(Keys::A, Keys::D, Keys::S, Keys::W);
		Math::Vec2 keyboardAxis2 = MInput::Keyboard()->GetAxis(Keys::Left, Keys::Right, Keys::Down, Keys::Up);
		Math::Vec2 direction = Calc::SafeNormalize(Calc::SafeNormalize(leftStick) + Calc::SafeNormalize(keyboardAxis1) + Calc::SafeNormalize(keyboardAxis2));
		if (MInput::Mouse()->CheckLeftButton()) {
			auto mouseMove = Math::Vec2(MInput::Mouse()->RawPositionDelta());
			mouseMove.y *= -1.f;
			direction = Calc::SafeNormalize(mouseMove + direction);
		}
		direction = Calc::SafeNormalize(direction);
		m_remainder += direction * speed * deltaTime;
		auto move = Math_Round(m_remainder);
		m_remainder -= move;
		static bool firstMove = true;
		if (move != Vec2_Zero && firstMove) {
			firstMove = false;
			s_level = 0;
			s_startTime = Engine::Instance()->GameTime();
			GetEntity()->GetSceneAs<MainScene>()->m_coverTween->Start();
			Audio::Resume(s_bgm);
		}
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

	GUI::Disable();
	InitSpriteBank();

	Graphics::SetRenderPassOrder({ 0, kBackgroundPass, kMainPass, kScreenTexturePass });

	Graphics::UseRenderPass(kBackgroundPass)->SetClearColor(Color::Black);
	Graphics::UseRenderPass(kMainPass)->SetClearDiscard();
	Graphics::UseRenderPass(kScreenTexturePass)->SetClearDiscard();

	Graphics::CreateUniformVec4("u_data");

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
	m_progressBar->Depth(-1);
	m_progressBar->PositionY(179.f);
	m_progressBar->Add(new ProgressGraphicsComponent);
	Add(m_progressBar);

	m_cover = new Entity();
	m_cover->Depth(-2);
	auto coverSprite = s_spriteBank->Create("cover");
	m_cover->Add(coverSprite);
	m_coverTween = Tween::Create(TweenMode::Persist, Ease::SineInOut, 1.5f, false);
	m_coverTween->OnUpdate([this](Tween* tween) { m_cover->PositionY(tween->Eased() * 180.f); });
	m_coverTween->OnComplete([coverSprite](Tween* tween) { coverSprite->Play("thanks"); });
	m_cover->Add(m_coverTween);
	Add(m_cover);

	auto screenTex = new Entity();
	auto screenSpaceQuad = new ScreenSpaceQuadGraphicsComponent(m_mainScreenTarget->GetTexture2D());
	screenTex->Tag(s_screenTextureTag);
	screenTex->Add(screenSpaceQuad);
	Add(screenTex);

	Audio::LoadFile("bgm", "assets/sounds/BulletHellTrialBGM.ogg");
	s_bgm = Audio::Loop("bgm");
	Audio::Stop(s_bgm);

	base::Begin();
}

void MainScene::Update()
{
	auto viewSize = Engine::Instance()->GetViewSize();
	int cameraScale = Math_Min(Math_Max(1.f, viewSize.x / (float)m_mainScreenTarget->Width()), \
		Math_Max(1.f, viewSize.y / (float)m_mainScreenTarget->Height()));
	m_screenTexRenderer->GetCamera()->Scale2D(Math::Vec2(cameraScale));

	if (!s_dead) {
		auto bullet = m_player->CollideFirst(s_bulletTag);
		if (bullet) {
			s_localTimeRate = 0.f;
			MInput::GamePads(0)->Rumble(30.f, 0.2f);
			s_dead = true;
		}
	}
	if (s_dead) {
		Math::Vec2 center = m_player->Position2D() / Math::Vec2(180.f);
		if (!Graphics::IsOriginBottomLeft()) {
			center.y = 1.f - center.y;
		}
		float t = s_deadTime * 2.f;
		float radius = t;
		if (t > 1.5f) {
			center = Math::Vec2(0.5f);
			radius = 3.f - t;
		}
		Math::Vec4 uniformData(center, radius, 0.f);
		Graphics::SetUniform("u_data", &uniformData);
		m_player->Get<CircleGraphicsComponent>()->SetColor(Color::Blue);
		s_deadTime += Engine::Instance()->RawDeltaTime();

		Audio::SetParam(s_bgm, 1.0, 1.0 - radius / 2.0, 0.0);

		if (radius < 0.f) {
			m_player->Position2D(Math::Vec2(90.f));
			m_player->Get<CircleGraphicsComponent>()->SetColor(Color::Yellow);
			s_bulletPool.Traverse(
				[](auto bullet)
				{
					bullet->RemoveSelf();
				});
			s_dead = false;
			s_deadTime = 0.f;
			s_localTimeRate = 1.f;
			s_startTime = Engine::Instance()->GameTime();
			m_progressBar->Get<ProgressGraphicsComponent>()->ResetProgress();
			Audio::Stop(s_bgm);
			Audio::SetParam(s_bgm, 1.0, 1.0, 0.0);
			Audio::Resume(s_bgm);
		}
	}

	base::Update();
}
