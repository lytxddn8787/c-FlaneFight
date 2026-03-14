#include <iostream>
#include <graphics.h>
#include <windows.h> 
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#include <vector>
#include <conio.h>
using namespace std;

//窗口大小
constexpr auto GroundWidth = 500;
constexpr auto GroundHeight = 720;
//
//// 播放背景音乐（WAV格式，循环播放）
//void PlayBackgroundMusic()
//{
//	// SND_ASYNC: 异步播放（不阻塞程序）
//	// SND_LOOP: 循环播放
//	// SND_FILENAME: 参数是文件名
//	PlaySound(TEXT("res\\background.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
//}
//
//// 停止背景音乐
//void StopBackgroundMusic()
//{
//	PlaySound(NULL, NULL, 0);
//}

//获取鼠标位置
bool PointInRect(int x, int y, RECT& r)
{
	return (r.left <= x && r.right >= x && r.top <= y && y <= r.bottom);
}

//飞机碰撞
bool RectCrashRect(RECT& r1, RECT& r2)
{
	RECT r;
	r.left = r1.left - (r2.right - r2.left);
	r.right = r1.right;
	r.top = r1.top - (r2.bottom - r2.top);
	r.bottom = r1.bottom;

	return (r.left < r2.left && r2.left <= r.right && r.top <= r2.top && r2.top <= r.bottom);
}

//开始界面
void Welcome()
{
	LPCTSTR title = _T("飞机大战");
	LPCTSTR tplay = _T("开始游戏");
	LPCTSTR texit = _T("退出游戏");

	RECT tplayr, texitr;
	setbkcolor(WHITE);
	cleardevice();
	settextstyle(60, 0, _T("黑体"));
	settextcolor(BLACK);
	outtextxy(GroundWidth / 2 - textwidth(title) / 2, GroundHeight / 10, title);

	settextstyle(40, 0, _T("黑体"));
	tplayr.left = GroundWidth / 2 - textwidth(tplay) / 2;
	tplayr.right = tplayr.left + textwidth(tplay);
	tplayr.top = GroundHeight / 5 * 2.5;
	tplayr.bottom = tplayr.top + textheight(tplay);

	texitr.left = GroundWidth / 2 - textwidth(texit) / 2;
	texitr.right = texitr.left + textwidth(texit);
	texitr.top = GroundHeight / 5 * 3;
	texitr.bottom = texitr.top + textheight(texit);

	outtextxy(tplayr.left, tplayr.top, tplay);
	outtextxy(texitr.left, texitr.top, texit);

	EndBatchDraw();

	while (true) {
		ExMessage mess;
		getmessage(&mess, EM_MOUSE);
		if (mess.lbutton) {
			if (PointInRect(mess.x, mess.y, tplayr))
				return ;
			else if (PointInRect(mess.x, mess.y, texitr))
				exit(0);
		}
	}
}

//结束界面
void Over(unsigned long long kills)
{
	TCHAR *str = new TCHAR[128];
	_stprintf_s(str, 128, _T("击杀数：%llu"), kills);
	settextcolor(RED);

	outtextxy(GroundWidth / 2 - textwidth(str) / 2, GroundHeight / 5,str);

	//按Enter返回
	LPCTSTR info = _T("按Entera返回");
	settextstyle(20, 0, _T("黑体"));
	outtextxy(GroundWidth - textwidth(info), GroundHeight - textheight(info), info);

	while (true) {
		ExMessage mess;
		getmessage(&mess, EM_KEY);
		if (mess.vkcode == 0X0D)
			return;

	}
}

//背景、敌机、英雄、子弹
class BK
{
public:
	BK(IMAGE& img) :img(img),y(-GroundHeight){}

	void Show() {
		if (y == 0) y = -GroundHeight;
		y += 3;
		putimage(0, y, &img);
	}

private:
	IMAGE& img;
	int y;
};

class Hero
{
public:
	Hero(IMAGE& img) :img(img) {
		rect.left = GroundWidth / 2 - img.getwidth() / 2;
		rect.top = GroundHeight - img.getheight();
		rect.right = rect.left + img.getwidth();
		rect.bottom = GroundHeight;
	}
	void Show() {
		putimage(rect.left, rect.top, &img);
	}
	void Control() {
		ExMessage mess;
		if (peekmessage(&mess, EM_MOUSE)) {
			rect.left = mess.x - img.getwidth() / 2;
			rect.top = mess.y - img.getheight() / 2;
			rect.right = rect.left + img.getwidth();
			rect.bottom = rect.top + img.getheight();
		}
	}
	RECT& GetRect() { return rect; }

private:
	IMAGE& img;
	RECT rect;
};

class Enemy
{
public:
	Enemy(IMAGE&img,int x):img(img){
		rect.left = x;
		rect.right = rect.left + img.getwidth();
		rect.top = -img.getheight();
		rect.bottom = 0;
	}
	bool Show() {
		if (rect.top >= GroundHeight) return false;
		rect.top += 4;
		rect.bottom += 4;
		putimage(rect.left, rect.top, &img);
		
		return true;
	}
	RECT& GetRect() { return rect; }

private:
	IMAGE& img;
	RECT rect;
};

class Bullet
{
public:
	Bullet(IMAGE& img, RECT pr) :img(img) {
		rect.left = pr.left + (pr.right - pr.left) / 2 - img.getwidth() / 2;
		rect.right = rect.left + img.getwidth();
		rect.top = pr.top - img.getheight();
		rect.bottom = rect.top + img.getheight();
	}
	bool Show() {
		if (rect.bottom <= 0)return false;
		rect.top -= 4;
		rect.bottom -= 4;
		putimage(rect.left, rect.top, &img);
	}
	RECT& GetRect() { return rect; }

private:
	IMAGE& img;
	RECT rect;
};

//生成敌机
bool AddEnemy(vector<Enemy*>&es,IMAGE&enemyimg)
{
	Enemy* e = new Enemy(enemyimg, abs(rand()) % (GroundWidth - enemyimg.getwidth()));
	for (auto& x : es) 
		if (RectCrashRect(x->GetRect(), e->GetRect())) {
			delete e;
			return false;
		}
	es.push_back(e);
	return true;
}

bool Play()
{
	//PlayBackgroundMusic();

	setbkcolor(WHITE);
	cleardevice();
	bool is_play = true;
	
	IMAGE heroimg, enemyimg, bkimg, bulletimg;
	loadimage(&heroimg, _T("res\\飞机.jpg"));
	loadimage(&enemyimg, _T("res\\敌机.jpg"));
	loadimage(&bulletimg, _T("res\\子弹.jpg"));
	loadimage(&bkimg, _T("res\\背景.jpg"), GroundWidth, GroundHeight * 2);

	BK bk = BK(bkimg);
	Hero hp = Hero(heroimg);

	vector<Enemy*>es;
	vector<Bullet*>bs;
	int bsing = 0;

	unsigned long long kills = 0;

	for (int i = 0; i < 5; i++)
		AddEnemy(es, enemyimg);
	
	while (is_play) {
		bsing++;
		if (bsing == 10) {
			bsing = 0;
			bs.push_back(new Bullet(bulletimg, hp.GetRect()));
		}

		BeginBatchDraw();

		bk.Show();
		Sleep(6);
		//暂停
		{
			ExMessage mess;
			if (peekmessage(&mess, EM_KEY)){
				if (mess.vkcode == 0x1B) {
					settextstyle(30, 0, _T("黑体"));
					settextcolor(RED);
					outtextxy(GroundWidth / 2 - 60, GroundHeight / 2, _T("暂停中"));
					outtextxy(GroundWidth / 2 - 100, GroundHeight / 2 + 40, _T("按空格键继续"));
					FlushBatchDraw();
					while (true) {
						flushmessage();
						getmessage(&mess, EM_KEY);
						if (mess.vkcode == 0x20)
							break;
					}
				}
			}
		}
		flushmessage();
		Sleep(1);
		hp.Control();
		//暂停
		//if (_kbhit()) {
		//	char v = _getch();
		//	if (v == ' ') {
		//		Sleep(500);
		//		// 暂停循环
		//		while (true) {
		//			if (_kbhit()) {
		//				char key = _getch();
		//				if (key == ' ') {
		//					break;
		//				}
		//			}
		//			Sleep(50);  // 减少CPU占用
		//		}
		//	}
		//}

		
		hp.Show();

		for (auto& x : bs)
			x->Show();

		auto it = es.begin();
		while (it != es.end()) {
			if (RectCrashRect((*it)->GetRect(),hp.GetRect()))
				is_play = false;
			auto bit = bs.begin();
			while (bit != bs.end()) {
				if (RectCrashRect((*bit)->GetRect(), (*it)->GetRect())) {
					delete(*it);
					es.erase(it);
					it = es.begin();
					delete(*bit);
					bs.erase(bit);
					kills++;
					break;
				}
				bit++;
			}
			if (!(*it)->Show()){
				delete(*it);
				es.erase(it);
				it = es.begin();
			}
			it++;
		}
		for (int i = 0; i < 5-es.size(); i++)
			AddEnemy(es, enemyimg);
		
		EndBatchDraw();
	}
	//StopBackgroundMusic();
	Over(kills);
	return true;
}

int main()
{
	//创建窗口
	initgraph(GroundWidth, GroundHeight, EW_NOMINIMIZE | EW_SHOWCONSOLE);
	//游戏运行
	bool is_live = true;
	while (is_live) {
		Welcome();
		//开始游戏
		is_live = Play();
	}


	return 0;
}