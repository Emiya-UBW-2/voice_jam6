#define FRAME_RATE 60.f
#include "DXLib_ref/DXLib_ref.h"

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
	float rt, rt_f;
public:
	int x, y;
	float col;
	bool the_end() {
		return rt >= (640.f - 100.f - 100.f);
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
		DrawCircle(x, y, int(r), GetColor(int(152.f*col/255.f), int(115.f*col / 255.f), int(169.f*col / 255.f)), TRUE);
		DrawCircle(x, y, int(rt), GetColor(255, 255, 255), FALSE, 3);
	}
	bool isHit(const cir& t) {
		if (this->col == 0.f || t.col == 0.f) {
			return false;
		}
		return (int(std::hypot((this->x - t.x), (this->y - t.y))) <= (this->r + t.r));
	}
};


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	auto Drawparts = std::make_unique<DXDraw>("FPS_0", 960, 640, FRAME_RATE);	/*汎用クラス*/
	auto HostPassparts = std::make_unique<HostPassEffect>(960, 640);	/*ホストパスエフェクトクラス*/
	auto Debugparts = std::make_unique<DeBuG>(FRAME_RATE);	/*デバッグクラス*/
	GraphHandle screen = GraphHandle::Make(960, 640);

	std::vector<cir> circles;
	float power_ = (640.f - 100.f - 100.f) / 2.f;
	unsigned char life = 3;

	char click = 0;
	int x_m, y_m;
	const int m_limit = 100;
	GetMousePoint(&x_m, &y_m);
	float x_dpos=0.f, y_dpos = 0.f,power_dpos = 0.f;
	float cnt_back = 0.f;


	while (ProcessMessage() == 0) {
		const float fps = GetFPS();
		const auto waits = GetNowHiPerformanceCount();
		Debugparts->put_way();
		if (life > 0) {
			//マウス座標処理
			{
				int x_o = x_m, y_o = y_m;
				GetMousePoint(&x_m, &y_m);
				x_m = std::clamp(x_m, 0 + m_limit, 960 - m_limit);
				y_m = std::clamp(y_m, 0 + m_limit, 640 - m_limit);
				SetMousePoint(x_m, y_m);
			}
			//クリック処理
			{
				click = std::clamp<char>(click + 1, 0, ((GetMouseInput()&MOUSE_INPUT_LEFT) != 0) ? 100 : 0);
				if (click == 1) {
					circles.resize(circles.size() + 1);
					circles.back().set(power_);
					power_ /= 4.f;
				}
				else {
					power_ = std::clamp(power_ + 100.f / GetFPS(), 0.f, (640.f - 100.f - 100.f) / 2.f);
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
							c.col = 0.f;
							if (life > 0) {
								life--;
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
		screen.SetDraw_Screen(true);
		{
			for (auto&c : circles) {
				c.draw();
			}
			DrawCircle(x_m, y_m, power_, GetColor(255,0,0), FALSE);
		}
		GraphHandle::SetDraw_Screen(int(DX_SCREEN_BACK), true);
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
			if (cnt_back<=0.5f) {
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
			DrawFormatString(0, 640 - 18, GetColor(255, 0, 0), "pow : %6.2f", power_);
			for (int i = 0; i < life; i++) {
				DrawBox(10 + i * 36, 640 - 72+i*8, 10 + i * 36 + 32, 640 - 36, GetColor(255, 0, 0), TRUE);
			}
			if (life == 0) {
				DrawString(100, 640 - 96, "GAME OVER", GetColor(255, 0, 0));
				DrawString(100, 640 - 72, "PRESS ESC KEY", GetColor(255, 0, 0));
			}
			//debug
			Debugparts->end_way();
			Debugparts->debug(10, 10, float(GetNowHiPerformanceCount() - waits) / 1000.f);
		}
		//画面更新
		Drawparts->Screen_Flip(waits);

		if (CheckHitKey(KEY_INPUT_ESCAPE) != 0) {
			break;
		}
	}
	return 0;				// ソフトの終了 
}