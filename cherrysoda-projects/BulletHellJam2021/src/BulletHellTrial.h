#include <CherrySoda/Engine.h>

namespace cherrysoda {

class SpriteBank;

} // namespace cherrysoda

namespace bullethelltrial {

class BulletHellTrial : public cherrysoda::Engine
{
public:
	typedef cherrysoda::Engine base;

	BulletHellTrial();

	void Initialize() override;
	void LoadContent() override;
	void UnloadContent() override;

	static cherrysoda::SpriteBank* GetSpriteBank() { return ms_spriteBank; }

private:
	static cherrysoda::SpriteBank* ms_spriteBank;
};

} // namespace bullethelltrial
