#include <CherrySoda/Engine.h>

namespace bullethelltrial {

class BulletHellTrial : public cherrysoda::Engine
{
public:
	typedef cherrysoda::Engine base;

	BulletHellTrial();

	void Update() override;
	void Initialize() override;
	void LoadContent() override;
};

} // namespace bullethelltrial
