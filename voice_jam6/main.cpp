#define FRAME_RATE 60.f
#define m_limit 100
#include "DXLib_ref/DXLib_ref.h"

int break_f = 0;
bool play_f = false;
int x_m, y_m;
GraphHandle screen;
FontHandle font24, font36;
std::vector<std::array<std::string, 3>> lang;

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
				font36.DrawString_MID(960 / 2, 640 / 4, lang[0][break_f], GetColor(255, 255, 255));

				font24.DrawString_MID(960 / 2, 640 / 2 + 24 * 0, lang[1][break_f], GetColor(255, 0, 0));
				font24.DrawString_MID(960 / 2, 640 / 2 + 24 * 1, lang[2][break_f], GetColor(255, 0, 0));

				font24.DrawString_MID(960 / 2, 640 / 2 + 24 * 3, lang[3][break_f], GetColor(255, 128, 0));
				font24.DrawString_MID(960 / 2, 640 / 2 + 24 * 4, lang[4][break_f], GetColor(255, 128, 0));

				font24.DrawString_MID(960 / 2, 640 * 3 / 4, lang[5][break_f], GetColor(255, 0, 0));
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
	float score_r = 0.f;
	std::vector<std::pair<int, float>> p_add;
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
		score_r = 0.f;
		p_add.clear();
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
					p_add.emplace_back(100 + int(6900.f*((100.f) - power_) / (100.f)), 1.f);
					score += p_add.back().first;
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
								p_add.emplace_back(-score, 1.f);
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
			if (break_f == 0) {
				DrawCircle(x_m, y_m, int(power_), GetColor(255, 0, 0), FALSE);
			}
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
				int digit = 10;
				{
					int num = std::abs(score - int(score_r));
					while (num != 0) {
						num /= 10;
						digit += 6;
					}
				}

				for (int i = 0; i < digit; i++) {
					if (score > int(score_r)) {
						score_r += 10.f / GetFPS();
					}
					else if (score < int(score_r)) {
						score_r -= 10.f / GetFPS();
					}
					else {
						break;
					}
				}

				font36.DrawStringFormat(200 + 1, 640 - 92 + 1, GetColor(0, 0, 0), lang[6][break_f], int(score_r));
				font36.DrawStringFormat(200, 640 - 92, GetColor(152, 115, 169), lang[6][break_f], int(score_r));
				for (auto&p : p_add) {
					SetDrawBlendMode(DX_BLENDMODE_ALPHA, int(255.f*p.second));
					if (p.first >= 0) {
						font24.DrawStringFormat(440, 640 - 92 - int((1.f - p.second)*36.f), GetColor(0, 0, 255), "+%d", p.first);
					}
					else {
						font24.DrawStringFormat(440 + 36, 640 - 92 - int((1.f - p.second)*36.f), GetColor(255, 0, 0), "-%d", p.first);
					}
					SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
				}

				while (true) {
					int i = 0;
					for (auto&p : p_add) {
						p.second -= 1.5f / GetFPS();
						if (p.second <= 0.f) {
							p_add.erase(p_add.begin() + i);
							break;
						}
						i++;
					}
					if (i == p_add.size()) {
						break;
					}
				}

				font36.DrawStringFormat(200 + 1, 640 - 56 + 1, GetColor(0, 0, 0), lang[7][break_f], power_);
				font36.DrawStringFormat(200, 640 - 56, GetColor(152, 115, 169), lang[7][break_f], power_);
				for (int i = 0; i < 3; i++) {
					DrawBox(10 + i * 54, 640 - 72 + i * 12, 10 + i * 54 + 48 + 1, 640 - 18 + 1, GetColor(0, 0, 0), TRUE);
					if (i < life) {
						DrawBox(10 + i * 54, 640 - 72 + i * 12, 10 + i * 54 + 48, 640 - 18, GetColor(152, 115, 169), TRUE);
					}
					DrawBox(10 + i * 54, 640 - 72 + i * 12, 10 + i * 54 + 48, 640 - 18, GetColor(128, 128, 128), FALSE);
				}
				if (life == 0) {
				}
			}
		}
		if (life == 0) {
			return true;
		}

		if (score >= 65535) {
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
				/*
				if (break_f <= 1 || (break_f == 2 && score >= 65535)) {
					font36.DrawStringFormat(960 / 7, 640 / 10, GetColor(234, 139, 21), lang[8][break_f], score);
					font24.DrawString_RIGHT(960 * 6 / 7, 640 - 96, lang[9][break_f], GetColor(255, 0, 0));
				}
				else {
					font36.DrawStringFormat(960 / 7, 640 / 10, GetColor(234, 139, 21), lang[8][1], score);
					font24.DrawString_RIGHT(960 * 6 / 7, 640 - 96, lang[9][1], GetColor(255, 0, 0));
				}
				*/
				if (score < 65535) {
					font36.DrawStringFormat(960 / 7, 640 / 10, GetColor(234, 139, 21), lang[8][break_f], score);
					font24.DrawString_RIGHT(960 * 6 / 7, 640 - 96, lang[9][break_f], GetColor(255, 0, 0));
				}
				else {
					font36.DrawStringFormat(960 / 7, 640 / 10, GetColor(234, 139, 21), lang[8][2], score);
					font24.DrawString_RIGHT(960 * 6 / 7, 640 - 96, lang[9][2], GetColor(255, 0, 0));
				}
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

void noise(int per = 60) {
	int p = 0, q = 0;
	for (int i = 0; i < per; i++) {
		p = GetRand(640);
		q = GetRand(960);
		DrawLine(q, p, (q < 960 / 2) ? 960 : 0, p, GetColor(255, 255, 255));
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	auto Drawparts = std::make_unique<DXDraw>("FPS_0", 960, 640, FRAME_RATE);	/*汎用クラス*/
	SetValidMousePointerWindowOutClientAreaMoveFlag(FALSE);
	auto titleparts = std::make_unique<title>();	/*タイトルクラス*/
	auto gameparts = std::make_unique<game>();	/*ゲームクラス*/
	auto resparts = std::make_unique<res>();	/*タイトルクラス*/
	//auto Debugparts = std::make_unique<DeBuG>(FRAME_RATE);	/*デバッグクラス*/

	lang.resize(lang.size() + 10);
	lang[0][0] = "GAME";
	lang[0][1] = "GAHA";
	lang[0][2] = "HELP";

	lang[1][0] = "クリックすると ワ が でます";
	lang[1][1] = "クリックすると た す けてす";
	lang[1][2] = "たすけてよると た す けてよ";

	lang[2][0] = "ワ が かさならないよう クリックしつづけよう！";
	lang[2][1] = "で が possible overflow warning! :(";
	lang[2][2] = "で ら れなn/a?ないよう たすけてよたすけてよ！";

	lang[3][0] = "パワーが ちいさいほど トクテン アップ！";
	lang[3][1] = "スコアが おおきいほど でられる アップ！";
	lang[3][2] = "たすけて たりないほど たすけて たすけて";

	lang[4][0] = "ワが カサなると トクテン はんげん...";
	lang[4][1] = "６５ ５３５  と トクテン はんげん...";
	lang[4][2] = "４ ２９４ ９６７ ２９５  はんげん...";

	lang[5][0] = "CLICK to START";
	lang[5][1] = "CLICK to START";
	lang[5][2] = "CLICK to START";

	lang[6][0] = "SCORE : %04d / 65535";
	lang[6][1] = "SCORE : %05d / 65535";
	lang[6][2] = "SCORE : %08d / 4294967295";

	lang[7][0] = "POWER : %6.2f";
	lang[7][1] = "POWER : %6.2f";
	lang[7][2] = "POWER : %6.2f";

	lang[8][0] = "SCORE : %04d";
	lang[8][1] = "SCORE : %04d";
	lang[8][2] = "GAME CLEAR";

	lang[9][0] = "CLICK to NEXT";
	lang[9][1] = "CLICK to NEXT";
	lang[9][2] = "continue?????";

	LPCSTR font_path = "x8y12pxTheStrongGamer.ttf"; // 読み込むフォントファイルのパス
	if (!(AddFontResourceEx(font_path, FR_PRIVATE, NULL) > 0)) {
		MessageBox(NULL, "フォント読込失敗", "", MB_OK);		// フォント読込エラー
	}
	font24 = FontHandle::Create("x8y12pxTheStrongGamer", 18);
	font36 = FontHandle::Create("x8y12pxTheStrongGamer", 36);

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
			//Debugparts->put_way();
			//update
			bool ed = false;
			switch (scene) {
			case 0:
				ed = titleparts->update();
				break;
			case 1:
				ed = gameparts->update();
				resparts->score = gameparts->score;
				break;
			case 2:
				ed = resparts->update();
				break;
			}
			//
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, int(bb));
			switch (break_f) {
			case 0:
				DrawBox(0, 0, 960, 640, GetColor(234, 139, 21), TRUE);
				break;
			case 1:
				DrawBox(0, 0, 960, 640, GetColor(200, 64, 0), TRUE);
				break;
			case 2:
				DrawBox(0, 0, 960, 640, GetColor(192, 0 , 0), TRUE);
				break;
			}
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
			easing_set(&bb, 0.f, 0.99f, GetFPS());
			//
			noise(break_f*30);
			//
			font24.DrawString_RIGHT(960 - 10 + 1, 640 - 24 - 10 + 1, "ESC EXIT", GetColor(0, 0, 0));
			font24.DrawString_RIGHT(960 - 10, 640 - 24 - 10, "ESC EXIT", GetColor(192, 192, 192));
			//debug
			//Debugparts->end_way();
			//Debugparts->debug(10, 10, float(GetNowHiPerformanceCount() - waits) / 1000.f);
			//画面更新
			Drawparts->Screen_Flip(waits);
			//強制終了
			if (CheckHitKey(KEY_INPUT_ESCAPE) != 0) {
				break;
			}
			//遷移
			if (ed) {
				play_f = true;
				for (int i = 0; i <= 60; i++) {
					const auto waits_ = GetNowHiPerformanceCount();
					GraphHandle::SetDraw_Screen(int(DX_SCREEN_BACK), false);

					SetDrawBlendMode(DX_BLENDMODE_ALPHA, 40);
					switch (break_f) {
					case 0:
						DrawBox(0, 0, 960, 640, GetColor(234, 139, 21), TRUE);
						break;
					case 1:
						DrawBox(0, 0, 960, 640, GetColor(200, 64, 0), TRUE);
						break;
					case 2:
						DrawBox(0, 0, 960, 640, GetColor(192, 0, 0), TRUE);
						break;
					}

					SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

					noise(break_f * 30);

					font24.DrawString_RIGHT(960 - 10 + 1, 640 - 24 - 10 + 1, "ESC EXIT", GetColor(0, 0, 0));
					font24.DrawString_RIGHT(960 - 10, 640 - 24 - 10, "ESC EXIT", GetColor(192, 192, 192));

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
					if (resparts->score >= 65535) {
						//break_f++;
						break_f = 4;
						scene = 0;
					}
					break;
				}
				break;
			}
		}
		if (scene == 0) {
			if (break_f == 3) {
				MessageBox(NULL, "YOU COULD NOT HELP HER HAHAHAHAHAHAHA", "", MB_OK);
			}
			break;
		}
	} while (!(CheckHitKey(KEY_INPUT_ESCAPE) != 0));
	if (!RemoveFontResourceEx(font_path, FR_PRIVATE, NULL)) {
		MessageBox(NULL, "remove failure", "", MB_OK);
	}

	if (!play_f) {
		//「ありがとう」
	}
	return 0;
}