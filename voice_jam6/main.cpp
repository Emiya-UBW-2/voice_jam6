#define FRAME_RATE 60.f
#define m_limit 100
#include "DXLib_ref/DXLib_ref.h"

int x_m, y_m;
GraphHandle screen;
FontHandle font18, font36;

class HostPassEffect {
private:
	GraphHandle GaussScreen;
	const int EXTEND = 8;
	int disp_x = 1920;
	int disp_y = 1080;
public:
	HostPassEffect(const int& xd, const int& yd) {
		disp_x = xd;
		disp_y = yd;
		GaussScreen = GraphHandle::Make(disp_x / EXTEND, disp_y / EXTEND); /*エフェクト*/
	}
	~HostPassEffect() {
	}
	//ブルームエフェクト
	void bloom(GraphHandle& BufScreen, const int& level = 255) {
		GraphFilter(BufScreen.get(), DX_GRAPH_FILTER_TWO_COLOR, 200, GetColor(0, 0, 0), 255, GetColor(128, 128, 128), 255);
		GraphFilterBlt(BufScreen.get(), GaussScreen.get(), DX_GRAPH_FILTER_DOWN_SCALE, EXTEND);
		GraphFilter(GaussScreen.get(), DX_GRAPH_FILTER_GAUSS, 16, 1000);
		SetDrawMode(DX_DRAWMODE_BILINEAR);
		SetDrawBlendMode(DX_BLENDMODE_ADD, level);
		GaussScreen.DrawExtendGraph(0, 0, disp_x, disp_y, false);
		GaussScreen.DrawExtendGraph(0, 0, disp_x, disp_y, false);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
		SetDrawMode(DX_DRAWMODE_NEAREST);
	}

public:
};
class cir {
private:
	float r, r_f;
	float rt;
	float col;
	int x, y;
public:
	bool the_end() {
		return rt >= (640.f - 100.f - 100.f);
	}

	void reset() {
		r_f = 0.f;
		r = 0.f;
		col = 255.f;
	}
	void set(float power) {
		GetMousePoint(&x, &y);
		r_f = power;
		col = 255.f;
	}

	void update() {
		easing_set(&r, r_f, 0.95f, GetFPS());
		if (col > 0.f) {
			rt = r;
		}
		else {
			easing_set(&rt, (640.f - 100.f - 100.f)*2.f, 0.95f, GetFPS());
		}
		col = std::clamp(col - 255.f / GetFPS() / 3.f, 0.f, 255.f);
	}
	void draw() {
		if (col > 0.f) {
			DrawCircle(x, y, int(r), GetColor(int(152.f*col / 255.f), int(115.f*col / 255.f), int(169.f*col / 255.f)), TRUE);
		}
		DrawCircle(x, y, int(rt), GetColor(int(255.f*(1.f - (rt - r) / (640.f - 100.f - 100.f - r))), int(255.f*(1.f - (rt - r) / (640.f - 100.f - 100.f - r))), int(255.f*(1.f - (rt - r) / (640.f - 100.f - 100.f - r)))), FALSE, 6);
	}
	bool isHit(const cir& t) {
		bool ans = false;
		if (this->col == 0.f || t.col == 0.f) {
			return ans;
		}
		ans = (int(std::hypot((this->x - t.x), (this->y - t.y))) <= (this->r + t.r));
		if (ans) {
			this->col = 0.f;
		}
		return ans;
	}
};

class title {
private:
	char click = 0;
	cir circles;
public:
	void set() {
		click = 0;
		circles.reset();
	}
	bool update() {
		GetMousePoint(&x_m, &y_m);
		//SetMousePoint(960 / 2, 640 / 2);
		if (((GetMouseInput()&MOUSE_INPUT_LEFT) != 0) & (click < 2)) {
			click++;
			if (click == 1) {
				circles.set(64);
			}
		}
		if (click != 0) {
			circles.update();
		}
		screen.SetDraw_Screen(true);
		{
			if (click != 0) {
				circles.draw();
			}
			DrawCircle(x_m, y_m, 64, GetColor(255, 0, 0), FALSE);
		}
		GraphHandle::SetDraw_Screen(int(DX_SCREEN_BACK), true);
		{
			screen.DrawGraph(0, 0, false);

			{
				font36.DrawString_MID(960 / 2, 640 / 4, "GAME", GetColor(255, 255, 255));

				font18.DrawString_MID(960 / 2, 640 * 3 / 4, "CLICK to START", GetColor(255, 0, 0));
			}
		}
		if (click != 0) {
			if (circles.the_end()) {
				return true;
			}
		}
		return false;
	}
};
class game {
private:
	std::unique_ptr< HostPassEffect, std::default_delete< HostPassEffect>> HostPassparts;

	unsigned char life = 3;
	std::vector<cir> circles;
	float power_ = (640.f - 100.f - 100.f) / 2.f;
	float x_dpos = 0.f, y_dpos = 0.f, power_dpos = 0.f;
	float cnt_back = 0.f;
	char click = 0;
public:
	int score = 0;
	game() {
		HostPassparts = std::make_unique<HostPassEffect>(960, 640);	/*ホストパスエフェクトクラス*/
	}
	void set() {
		life = 3;
		circles.clear();
		power_ = (640.f - 100.f - 100.f) / 2.f;
		x_dpos = 0.f;
		y_dpos = 0.f;
		power_dpos = 0.f;
		cnt_back = 0.f;
		score = 0;
	}
	bool update() {
		if (life > 0) {
			//マウス座標処理
			{
				int x_o = x_m, y_o = y_m;
				GetMousePoint(&x_m, &y_m);
				x_m = std::clamp(x_m, 0 + m_limit + int(power_), 960 - m_limit - int(power_));
				y_m = std::clamp(y_m, 0 + m_limit + int(power_), 640 - m_limit - int(power_));
				SetMousePoint(x_m, y_m);
			}
			//クリック処理
			{
				click = std::clamp<char>(click + 1, 0, ((GetMouseInput()&MOUSE_INPUT_LEFT) != 0) ? 100 : 0);
				if (click == 1) {
					circles.resize(circles.size() + 1);
					circles.back().set(power_);
					score += 10 + int(990.f*((100.f) - power_) / (100.f));
					power_ /= 3.f;
				}
				else {
					power_ = std::clamp(power_ + 100.f / GetFPS(), 0.f, 100.f);
				}
			}
		}
		//演算
		{
			easing_set(&x_dpos, cos(deg2rad(GetRand(360)))*power_dpos, 0.95f, GetFPS());
			easing_set(&y_dpos, sin(deg2rad(GetRand(360)))*power_dpos, 0.95f, GetFPS());
			easing_set(&power_dpos, 0.f, 0.9f, GetFPS());

			for (auto&c : circles) {
				c.update();
				for (auto&t : circles) {
					if (&c != &t) {
						if (c.isHit(t)) {
							power_dpos = 64.f;
							if (life > 0) {
								life--;
								score /= 2;
							}
						}
					}
				}
			}
		}
		//消去処理
		{
			for (int i = 0; i < circles.size(); i++) {
				if (circles[i].the_end()) {
					circles.erase(circles.begin() + i);
					break;
				}
			}
		}
		{
			if (power_dpos >= 1.f) {
				cnt_back += 4.f / GetFPS();
				if (cnt_back >= 1.f) {
					cnt_back = 0.f;
				}
			}
			else {
				cnt_back = 0.f;
			}
		}
		//描画
		screen.SetDraw_Screen(true);
		{
			for (auto&c : circles) {
				c.draw();
			}
			DrawCircle(x_m, y_m, int(power_), GetColor(255, 0, 0), FALSE);
		}
		GraphHandle::SetDraw_Screen(int(DX_SCREEN_BACK), true);
		{
			if (cnt_back <= 0.5f) {
				DrawBox(0, 0, 960, 640, GetColor(234, 139, 21), TRUE);
			}
			else {
				DrawBox(0, 0, 960, 640, GetColor(255, 0, 0), TRUE);
			}
			screen.DrawRectGraph(
				0 + m_limit + int(x_dpos), 0 + m_limit + int(y_dpos),
				0 + m_limit + int(x_dpos), 0 + m_limit + int(y_dpos),
				960 - m_limit - m_limit + int(x_dpos), 640 - m_limit - m_limit + int(y_dpos),
				false);
			HostPassparts->bloom(screen);
			//UI
			{
				DrawFormatString(200, 640 - 36, GetColor(152, 115, 169), "score : %04d", score);
				DrawFormatString(200, 640 - 18, GetColor(152, 115, 169), "pow   : %6.2f", power_);
				for (int i = 0; i < 3; i++) {
					DrawBox(10 + i * 36, 640 - 72 + i * 8, 10 + i * 36 + 32, 640 - 36, GetColor(0,0,0), TRUE);
					if (i < life) {
						DrawBox(10 + i * 36, 640 - 72 + i * 8, 10 + i * 36 + 32, 640 - 36, GetColor(152, 115, 169), TRUE);
					}
					DrawBox(10 + i * 36, 640 - 72 + i * 8, 10 + i * 36 + 32, 640 - 36, GetColor(128,128,128), FALSE);
				}
				if (life == 0) {
				}
			}
		}
		if (life == 0) {
			return true;
		}
		return false;
	}
};
class res {
private:
	char click = 0;
	cir circles;
public:
	int score = 0;
	void set() {
		click = 0;
		circles.reset();
	}
	bool update() {
		GetMousePoint(&x_m, &y_m);
		if (((GetMouseInput()&MOUSE_INPUT_LEFT) != 0) & (click < 2)) {
			click++;
			if (click == 1) {
				circles.set(64);
			}
		}
		if (click != 0) {
			circles.update();
		}
		screen.SetDraw_Screen(true);
		{
			if (click != 0) {
				circles.draw();
			}
			DrawCircle(x_m, y_m, 64, GetColor(255, 0, 0), FALSE);
		}
		GraphHandle::SetDraw_Screen(int(DX_SCREEN_BACK), true);
		{
			screen.DrawGraph(0, 0, false);

			{
				font36.DrawStringFormat(960/7, 640 / 10, GetColor(234, 139, 21), "SCORE : %04d", score);
				font18.DrawString_RIGHT(960*6/7, 640 - 96, "CLICK to NEXT", GetColor(255, 0, 0));
			}
		}
		if (click != 0) {
			if (circles.the_end()) {
				return true;
			}
		}
		return false;
	}
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	auto Drawparts = std::make_unique<DXDraw>("FPS_0", 960, 640, FRAME_RATE);	/*汎用クラス*/
	SetValidMousePointerWindowOutClientAreaMoveFlag(FALSE);
	auto titleparts = std::make_unique<title>();	/*タイトルクラス*/
	auto gameparts = std::make_unique<game>();	/*ゲームクラス*/
	auto resparts = std::make_unique<res>();	/*タイトルクラス*/
	auto Debugparts = std::make_unique<DeBuG>(FRAME_RATE);	/*デバッグクラス*/
	font18 = FontHandle::Create(18);
	font36 = FontHandle::Create(36);
	screen = GraphHandle::Make(960, 640);
	int scene = 0;
	float bb = 255.f;
	do {
		switch (scene) {
		case 0:
			titleparts->set();
			break;
		case 1:
			gameparts->set();
			resparts->score = gameparts->score;
			break;
		case 2:
			resparts->set();
			break;
		}
		GetMousePoint(&x_m, &y_m);
		while (ProcessMessage() == 0) {
			const float fps = GetFPS();
			const auto waits = GetNowHiPerformanceCount();
			Debugparts->put_way();
			//update
			bool ed = false;
			switch (scene) {
			case 0:
				ed = titleparts->update();
				break;
			case 1:
				ed = gameparts->update();
				resparts->score=gameparts->score;
				break;
			case 2:
				ed = resparts->update();
				break;
			}
			//
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, bb);
			DrawBox(0, 0, 960, 640, GetColor(234, 139, 21), TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
			easing_set(&bb, 0.f, 0.95f, GetFPS());
			//
			font18.DrawString_RIGHT(960-2, 640-18-2, "ESC EXIT", GetColor(192, 192, 192));
			//debug
			Debugparts->end_way();
			//Debugparts->debug(10, 10, float(GetNowHiPerformanceCount() - waits) / 1000.f);
			//画面更新
			Drawparts->Screen_Flip(waits);
			//強制終了
			if (CheckHitKey(KEY_INPUT_ESCAPE) != 0) {
				break;
			}
			//遷移
			if (ed) {
				SetDrawBlendMode(DX_BLENDMODE_ALPHA, 26);
				for(int i=0;i<=60;i++){
					const auto waits_ = GetNowHiPerformanceCount();
					GraphHandle::SetDraw_Screen(int(DX_SCREEN_BACK), false);
					DrawBox(0, 0, 960, 640, GetColor(234, 139, 21), TRUE);
					Drawparts->Screen_Flip(waits_);
				}
				SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
				bb = 255.f;
				switch (scene) {
				case 0:
					scene = 1;
					break;
				case 1:
					scene = 2;
					break;
				case 2:
					scene = 1;
					break;
				}
				break;
			}
		}
	} while (!(CheckHitKey(KEY_INPUT_ESCAPE) != 0));
	return 0;
}