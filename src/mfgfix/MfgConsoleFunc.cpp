#include "MfgConsoleFunc.h"
#include "BSFaceGenAnimationData.h"

namespace MfgFix::MfgConsoleFunc
{
	namespace
	{
		enum Mode : std::int32_t
		{
			Reset = -1,
			Phoneme,
			Modifier,
			ExpressionValue,
			ExpressionId
		};
	}

	std::uint32_t GetActiveExpression(const BSFaceGenAnimationData& a_animData)
	{
		std::uint32_t expression = BSFaceGenAnimationData::Expression::MoodNeutral;

		for (std::uint32_t i = 0; i < a_animData.expression1.count; ++i) {
			if (a_animData.expression1.values[i] > a_animData.expression1.values[expression]) {
				expression = i;
			}
		}

		return expression;
	}

	bool SetPhonemeModifier(RE::StaticFunctionTag*, RE::Actor* a_actor, std::int32_t a_mode, std::uint32_t a_id, std::int32_t a_value)
	{
		if (!a_actor) {
			return false;
		}

		auto animData = reinterpret_cast<BSFaceGenAnimationData*>(a_actor->GetFaceGenAnimationData());

		if (!animData) {
			return false;
		}

		RE::BSSpinLockGuard locker(animData->lock);

		switch (a_mode) {
		case Mode::Reset: {
			animData->ClearExpressionOverride();
			animData->Reset(0.0f, true, true, true, false);
			return true;
		}
		case Mode::Phoneme: {
			animData->phoneme2.SetValue(a_id, std::clamp(a_value, 0, 100) / 100.0f);
			return true;
		}
		case Mode::Modifier: {
			animData->modifier2.SetValue(a_id, std::clamp(a_value, 0, 100) / 100.0f);
			return true;
		}
		}

		return false;
	}

	void SmoothSetPhoneme(BSFaceGenAnimationData* animData, int a_id, int a_value)
	{
		int t1 = animData->phoneme2.count ? static_cast<int>(std::lround(animData->phoneme2.values[a_id] * 100.0f)) : 0;
		int t2 = 0;
		const int speed = 5;

		while (t1 != a_value) {
			t2 = (a_value - t1) / std::abs(a_value - t1);
			t1 += t2 * speed;

			if ((a_value - t1) / t2 < 0)
			{
				t1 = a_value;
			}

			animData->phoneme2.SetValue(a_id, std::clamp(a_value, 0, 100) / 100.0f);
		}
	}

	int randomInt(int min, int max)
	{
		return min + rand() % (max - min + 1);
	}

	void SmoothSetModifier(BSFaceGenAnimationData* animData, int mod1, int mod2, int str_dest)
	{
		int speed_blink_min = 25;
		int speed_blink_max = 60;
		int speed_eye_move_min = 5;
		int speed_eye_move_max = 15;
		int speed_blink = 0;

		int t1 = animData->modifier2.count ? std::lround(animData->modifier2.values[mod1] * 100.0f) : 0;

		int t2;
		int t3;
		int speed;

		if (mod1 < 2) {
			if (str_dest > 0) {
				speed_blink = randomInt(speed_blink_min, speed_blink_max);
				speed = speed_blink;
			}
			else {
				if (speed_blink > 0) {
					speed = static_cast<int>(std::round(speed_blink * 0.5));
				}
				else {
					speed = static_cast<int>(std::round(randomInt(speed_blink_min, speed_blink_max) * 0.5));
				}
			}
		}
		else if (mod1 > 7 && mod1 < 12) {
			speed = randomInt(speed_eye_move_min, speed_eye_move_max);
		}
		else {
			speed = 5;
		}

		while (t1 != str_dest) {
			t2 = (str_dest - t1) / std::abs(str_dest - t1);
			t1 += t2 * speed;

			if ((str_dest - t1) / t2 < 0) {
				t1 = str_dest;
			}
			//simulate identical time for both e.g. brows change
			if (!(mod2 < 0 || mod2 > 13)) {
				t3 = randomInt(0, 1);
				animData->modifier2.SetValue(mod1 * t3 + mod2 * (1 - t3), std::clamp(t1, 0, 100) / 100.0f);
				animData->modifier2.SetValue(mod2 * t3 + mod1 * (1 - t3), std::clamp(t1, 0, 100) / 100.0f);

			}
			else {
				animData->modifier2.SetValue(mod1, std::clamp(t1, 0, 100) / 100.0f);
			}
		}
	}

	bool SetPhonemeModifierSmooth(RE::StaticFunctionTag*, RE::Actor* a_actor, std::int32_t a_mode, std::uint32_t a_id1, std::uint32_t a_id2, std::int32_t a_value)
	{
		if (!a_actor) {
			return false;
		}

		auto animData = reinterpret_cast<BSFaceGenAnimationData*>(a_actor->GetFaceGenAnimationData());

		if (!animData) {
			return false;
		}

		RE::BSSpinLockGuard locker(animData->lock);

		std::srand(static_cast<unsigned int>(std::time(nullptr)));

		switch (a_mode) {
		case Mode::Reset: {
			animData->ClearExpressionOverride();
			animData->Reset(0.0f, true, true, true, false);
			return true;
		}
		case Mode::Phoneme: {
			SmoothSetPhoneme(animData, a_id1, a_value);
			return true;
		}
		case Mode::Modifier: {
			SmoothSetModifier(animData, a_id1, a_id2, a_value);
			return true;
		}
		}

		return false;
	}
	//bool function SmoothSetModifier(Actor act, Int mod1, Int mod2, Int str_dest, float strModifier) native global
	//		bool function SmoothSetPhoneme(Actor act, Int number, Int str_dest, float modifier) native global

	std::int32_t GetPhonemeModifier(RE::StaticFunctionTag*, RE::Actor* a_actor, std::int32_t a_mode, std::uint32_t a_id)
	{
		if (!a_actor) {
			return -1;
		}

		auto animData = reinterpret_cast<BSFaceGenAnimationData*>(a_actor->GetFaceGenAnimationData());

		if (!animData) {
			return -1;
		}

		RE::BSSpinLockGuard locker(animData->lock);

		switch (a_mode) {
		case Mode::Phoneme: {
			return a_id < animData->phoneme2.count ? std::lround(animData->phoneme2.values[a_id] * 100.0f) : 0;
		}
		case Mode::Modifier: {
			return a_id < animData->modifier2.count ? std::lround(animData->modifier2.values[a_id] * 100.0f) : 0;
		}
		case Mode::ExpressionValue: {
			return a_id < animData->expression1.count ? std::lround(animData->expression1.values[a_id] * 100.0f) : 0;
		}
		case Mode::ExpressionId: {
			return GetActiveExpression(*animData);
		}
		}

		return -1;
	}

	void Register()
	{
		SKSE::GetPapyrusInterface()->Register([](RE::BSScript::IVirtualMachine* a_vm) {
			a_vm->RegisterFunction("SetPhonemeModifier", "MfgConsoleFunc", SetPhonemeModifier);
			a_vm->RegisterFunction("GetPhonemeModifier", "MfgConsoleFunc", GetPhonemeModifier);
			a_vm->RegisterFunction("SetPhonemeModifierSmooth", "MfgConsoleFunc", SetPhonemeModifierSmooth);
			return true;
			});
	}
}
