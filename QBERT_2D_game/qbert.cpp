#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

#include <map>
#include <iostream>
#include <vector>
#include <list>
#include <array>
#include <time.h>
#include  <string>
#include <fstream>
#include <thread>
#include <chrono>

#define ScreenWidth 1500
#define ScreenHeight 800
#define FPS 60
#define GRAVITY 3.0

typedef signed char offset_t;
typedef unsigned short animid_t;
typedef float speed;


enum Dir{ RU, RD, LD, LU, NONE };

Dir RandomDownDir(){
	byte rd = rand() % 2;
	if (rd == 0)
		return RD;
	else
		return LD;
}

class delay_q;
class tile;
class TerrainFilm;
enum tileColor{ green, yellow };
class platform;

class tile{
	float x, y;
	float post_x, post_y;
	tileColor color;
public:
	tile(float x, float y, float post_x, float post_y) : x(x), y(y), post_x(post_x), post_y(post_y), color(green){}
	float getx(){ return x; }
	float gety(){ return y; }
	float getpost_x(){ return post_x; }
	float getpost_y(){ return post_y; }
	tileColor getcolor(){ return color; }

	void setcolor(tileColor color){ this->color = color; }
};
class delay_q{
	bool blocked;
	unsigned int delay;
public:
	delay_q() :delay(0), blocked(false){}
	delay_q(unsigned int delay) :delay(delay), blocked(false){}
	void operator()(unsigned int d){
		delay = d;
	}
	bool operator()(){
		if (blocked)
			return true;
		if (delay == 0){
			return false;
		}
		else {
			--this->delay;
			return true;
		}
	}
	unsigned int getdelay(){ return this->delay; }
	void block(){ blocked = true; }
	void unblock(){ blocked = false; }
	bool isBlocked(){ return blocked; }
};

class PlatformFilm{
private:
	ALLEGRO_BITMAP* platform;
	float width, height;
public:
	PlatformFilm(){
		platform = al_load_bitmap("platform.png");
		if (!platform){
			std::cerr << "platform.png not found or failed to load\n";
			exit(1);
		}
		width = al_get_bitmap_width(platform);
		height = al_get_bitmap_height(platform);
	}
	~PlatformFilm(){
		al_destroy_bitmap(platform);
	}
	ALLEGRO_BITMAP* getplatform(){
		return this->platform;
	}
	float getWidth(){ return this->width; }
	float getHeight(){ return this->height; }

};

class platform{
	float x, y;
	float post_x, post_y;
	speed sp;
	delay_q delay;
	bool visible;
public:
	void setplatform(float x, float y, unsigned int delay, speed sp, bool visible) {
		this->sp = sp;
		this->x = x;
		this->y = y;
		this->post_x = x + 75 / 2;
		this->post_y = y + 15;
		this->delay(delay);
		this->visible = visible;
	}
	void setplatform(float x, float y){
		this->x = x;
		this->y = y;
		this->post_x = x + 75 / 2;
		this->post_y = y + 15;
	}
	float getx(){ return x; }
	float gety(){ return y; }
	float getpost_x(){ return post_x; }
	float getpost_y(){ return post_y; }
	bool run_delay(){
		return delay();
	}

	//void setxy(float x, float y){ x = x; y = y; }
	//void setpost_xy(float post_x, float post_y){ post_x = post_x; post_y = post_y; };
	speed getspeed(){ return sp; }
	void setspeed(speed sp){ this->sp = sp; }
	void set_delay(unsigned int d){ delay(d); }
	bool isVisible(){ return visible; }
	void hide(){ visible = false; }
	void show(){ visible = true; }

};


class observeTerrain{
public:
	virtual void observeT(int levels, float tilewidth, float tileheight, std::map< int, std::vector<tile> >  *TilesCords){}
	virtual void observeRP(platform rp){}
	virtual void observeLP(platform lp){}
};
class observeQBert{
public:
	virtual void observeQB(int sprites_num, float spriteWidth, float spriteHeight){}
};

class controler :observeTerrain, observeQBert {
private:
	std::vector<std::array<float, 2>> RplatformPath;
	std::vector<std::array<float, 2>> LplatformPath;
	std::map< int, std::vector<std::array<float, 2>> >  QBPositions;
	std::map< int, std::vector<std::array<float, 2>> >  SnakePositions;
	float l;
	unsigned int totaltiles;
protected:
	std::map< int, std::vector<tile> >  *piramidTilesCoords;
	int terrain_levels;
	int tiles_num;
	float tilewidth, tileheight;

	int sprites_num;
	float spriteWidth;
	float spriteHeight;

public:
	void observeT(int levels, float tilewidth, float tileheight, std::map< int, std::vector<tile> >  *TilesCoords){
		this->terrain_levels = levels;
		this->tilewidth = tilewidth;
		this->tileheight = tileheight;
		piramidTilesCoords = TilesCoords;
		tiles_num = (levels + 1)*levels / 2;
		//to do check x/0 0/x
		if (this->terrain_levels > 1)
			this->l = (-1)*(((*piramidTilesCoords)[1][1].getx()) - ((*piramidTilesCoords)[0][0].getx())) / (((*piramidTilesCoords)[0][0].gety()) - ((*piramidTilesCoords)[1][1].gety()));
	}
	void observeQB(int sprites_num, float spriteWidth, float spriteHeight){
		this->spriteHeight = spriteHeight;
		this->spriteWidth = spriteWidth;
		this->sprites_num = sprites_num;
	}
	void observeRP(platform rp){
		float x = rp.getx();
		float y = rp.gety();
		while (x>(*piramidTilesCoords)[0][0].getpost_x() - 32){
			std::array<float, 2> tmp_array;
			x -= this->l*rp.getspeed();
			y -= rp.getspeed();
			tmp_array[0] = x;
			tmp_array[1] = y;
			RplatformPath.push_back(tmp_array);
		}
	}
	void observeLP(platform lp){
		float x = lp.getx();
		float y = lp.gety();
		while (x<(*piramidTilesCoords)[0][0].getx()){
			std::array<float, 2> tmp_array;
			x += this->l*lp.getspeed();
			y -= lp.getspeed();
			tmp_array[0] = x;
			tmp_array[1] = y;
			LplatformPath.push_back(tmp_array);
		}
	}

	void initializeControler(){
		assert(piramidTilesCoords, "piramidTilesCoords must be not NULL");
		for (int i = 0; i < this->terrain_levels; ++i){
			std::vector<std::array<float, 2>> Qtmp, SnakeTmp;
			for (size_t c = 0; c < (*piramidTilesCoords)[i].size(); ++c){
				std::array<float, 2> tmp_array = { (*piramidTilesCoords)[i][c].getpost_x() - control.getspriteWidth() / 2, (*piramidTilesCoords)[i][c].getpost_y() - control.getspriteHeight() };
				std::array<float, 2> tmp_array2 = { (*piramidTilesCoords)[i][c].getpost_x() - control.getspriteWidth() / 2, ((*piramidTilesCoords)[i][c].getpost_y() - control.getspriteHeight()) - 25 };

				Qtmp.push_back(tmp_array);
				SnakeTmp.push_back(tmp_array2);
			}
			QBPositions[i] = Qtmp;
			SnakePositions[i] = SnakeTmp;
		}

		//to do check x/0 0/x
		//if (this->terrain_levels > 1)
		//this->l = (-1.0)*(this->getQbposition_x(1, 1) - this->getQbposition_x(0, 0) ) / (this->getQbposition_y(0, 0) - this->getQbposition_y(1, 1));
		//std::cout << "-------->init" << this->l;
	}
	int getterrain_levels(){ return terrain_levels; }
	int gettiles_num(){ return tiles_num; }
	std::vector<tile> getTilesOfLevel(int level){
		assert(level<terrain_levels, "out of levels range\n");
		return (*piramidTilesCoords)[level];
	}
	tile* getTile(int level, int pos){
		assert(level < terrain_levels && pos <= level, "out of range\n");
		return &(*piramidTilesCoords)[level][pos];
	}
	int getsprite_num(){ sprites_num; }
	float getspriteWidth(){ return spriteWidth; }
	float getspriteHeight(){ return spriteHeight; }

	float getQbposition_x(int level, int pos){
		return this->QBPositions[level][pos][0];
	}
	float getQbposition_y(int level, int pos){
		return this->QBPositions[level][pos][1];
	}
	float getSnakeposition_x(int level, int pos){
		return this->SnakePositions[level][pos][0];
	}
	float getSnakeposition_y(int level, int pos){
		return this->SnakePositions[level][pos][1];
	}
	std::vector<std::array<float, 2>> getSnakeLevelpositions(int level){
		assert(level >= 0 && level<SnakePositions.size(), "whts that???");
		return this->SnakePositions[level];
	}
	std::vector<std::array<float, 2>> getRplatformPath(){ return this->RplatformPath; }
	std::vector<std::array<float, 2>> getLplatformPath(){ return this->LplatformPath; }
	std::vector<std::array<float, 2>> getQbLevelpositions(int level){
		assert(level >= 0 && level<QBPositions.size(), "whts that???");
		return this->QBPositions[level];
	}
	float getl(){ return l; }
}control;

class Score{
	std::string sc;
public:
	Score() :sc("0"){}
	void setScore(int points){
		sc = std::to_string(points);
	}
	void increase(int points){
		int i = std::stoi(sc);
		sc = std::to_string(i + points);
	}
	void decrease(int points){
		int i = std::stoi(sc);
		sc = std::to_string(i - points);
	}
	void reset(){ sc = "0"; }
	const char* getSScore(){ return sc.c_str(); }
	int getIScore(){ return std::stoi(sc); }
};

class TerrainFilm{
private:
	ALLEGRO_BITMAP* terrainTiles;
	float width, height;
public:
	TerrainFilm(){
		terrainTiles = al_load_bitmap("terrain0.png");
		if (!terrainTiles){
			std::cerr << "terrain0.png not found or failed to load\n";
			exit(1);
		}
		width = al_get_bitmap_width(terrainTiles);
		height = al_get_bitmap_height(terrainTiles);
	}
	~TerrainFilm(){
		al_destroy_bitmap(terrainTiles);
	}
	ALLEGRO_BITMAP* getTerainTiles(){
		return this->terrainTiles;
	}
	float getWidth(){ return this->width; }
	float getHeight(){ return this->height; }

};


class Terrain{
private:
	TerrainFilm tiles;
	PlatformFilm platformfilm;
	Score score;
	float topx;
	float topy;
	int levels;
	float TerHeight;

	platform Rplatform;
	platform Lplatform;
	float sprPlatformWidth;
	float sprPlatformHeight;
	float Rplatform_x, Rplatform_y;
	float Lplatform_x, Lplatform_y;
	int platformLevel;

	std::map< int, std::vector<tile> >  piramidTilesCoords;
	int YellowTilesNum;

	observeTerrain *observer;
protected:
	void drawYellowTile(float x, float y){
		al_draw_bitmap_region(tiles.getTerainTiles(), 0, 0, tiles.getWidth() / 2, tiles.getHeight(), x, y, NULL);
	}
	void drawGreenTile(float x, float y){
		al_draw_bitmap_region(tiles.getTerainTiles(), tiles.getWidth() / 2, 0, tiles.getWidth() / 2, tiles.getHeight(), x, y, NULL);
	}
	void drawTile(int level, int pos){
		tile *t = &piramidTilesCoords[level][pos];
		if (t->getcolor() == green){
			drawGreenTile(t->getx(), t->gety());
		}
		else{
			drawYellowTile(t->getx(), t->gety());
		}
	}
public:
	Terrain(int levels, controler* cnt) :topx(1500 / 2), topy(150), levels(levels), YellowTilesNum(0){
		this->observer = (observeTerrain*)cnt;  //set observer
		al_convert_mask_to_alpha(tiles.getTerainTiles(), al_map_rgb(0, 255, 254));      //backround trasparency
		al_convert_mask_to_alpha(platformfilm.getplatform(), al_map_rgb(0, 255, 255));  //backround trasparency
		this->TerHeight = levels*tiles.getHeight() - (levels - 1) * 15;	//set Terrain height
		/*set piramidTilesCoords*/
		int level = 0;
		while (level < levels){
			std::vector<tile> tile_vec;
			for (size_t i = 0; i <= level; i++)
			{
				float x = topx - level * 75 / 2 + i * 75;
				float y = topy + level*(75 - 15);
				float post_x = x + 75 / 2;
				float post_y = y + 15;
				tile t(x, y, post_x, post_y);
				tile_vec.push_back(t);
			}
			piramidTilesCoords[level] = tile_vec;
			tile_vec.clear();
			++level;
		}
		/*------------------------------------*/
		/*set platforms at 5th level if total levels >= 7*/
		if (levels >= 7){
			int li = -1, ri = 6 - 1;
			level = 5 - 1;
			float lx = topx - level * 75 / 2 + li * 75, rx = topx - level * 75 / 2 + ri * 75;
			float ly, ry;
			ly = ry = topy + level*(75 - 15);

			this->Rplatform_x = rx;
			this->Rplatform_y = ry;
			//Rplatform.setxy(rx,ry);
			this->Lplatform_x = lx;
			this->Lplatform_y = ly;
			//Lplatform.setxy(lx, ly);
			this->platformLevel = 5;
			Lplatform.setplatform(lx, ly, 5, 5, true);
			Rplatform.setplatform(rx, ry, 5, 5, true);
		}
		this->sprPlatformWidth = this->platformfilm.getWidth() / 4;
		this->sprPlatformHeight = this->platformfilm.getHeight();
		/*----------------------------*/
		observer->observeT(levels, tiles.getWidth(), tiles.getHeight(), &piramidTilesCoords); //controler observes terrain
		observer->observeRP(Rplatform);
		observer->observeLP(Lplatform);
	}

	void showTop(){
		al_draw_filled_circle(topx, topy, 10, al_map_rgba_f(.6, 0, .6, .6));

	}
	void drawPiramide(){
		for (int i = 0; i < levels; ++i){
			int size = piramidTilesCoords[i].size();
			for (int j = 0; j < size; ++j)
				drawTile(i, j);
		}
	}

	void drawRplatform(){
		if (Rplatform.isVisible()){
			static int i = 3;
			if (i == -1)
				i = 3;
			al_draw_bitmap_region(platformfilm.getplatform(), i*platformfilm.getWidth() / 4, 0, platformfilm.getWidth() / 4, platformfilm.getHeight(), Rplatform.getx(), Rplatform.gety(), NULL);
			if (!Rplatform.run_delay()){
				--i;
				Rplatform.set_delay(5);
			}
		}
	}
	void drawLplatform(){
		if (Lplatform.isVisible()){
			static int i = 3;
			if (i == -1)
				i = 3;
			al_draw_bitmap_region(platformfilm.getplatform(), i*platformfilm.getWidth() / 4, 0, platformfilm.getWidth() / 4, platformfilm.getHeight(), Lplatform.getx(), Lplatform.gety(), NULL);
			if (!Lplatform.run_delay()){
				--i;
				Lplatform.set_delay(5);
			}
		}
	}
	void setTileColor(tileColor color, int level, int pos){
		piramidTilesCoords[level][pos].setcolor(color);
	}
	tileColor getTileColor(int level, int pos){ return piramidTilesCoords[level][pos].getcolor(); }
	void setRplatform(float x, float y){ this->Rplatform.setplatform(x, y); }
	void setLplatform(float x, float y){ this->Lplatform.setplatform(x, y); }
	void YellowTilesIncrement(){ ++YellowTilesNum; }

	int getYellowTilesNum(){ return YellowTilesNum; }
	float getPlatformWidth(){ return this->sprPlatformWidth; }
	float getPlatformHeight(){ return this->sprPlatformHeight; }
	float getRplatform_x(){ return Rplatform_x; }
	float getRplatform_y(){ return Rplatform_y; }
	float getLplatform_x(){ return Lplatform_x; }
	float getLplatform_y(){ return Lplatform_y; }
	float getTopx(){ return topx; }
	float getTopy(){ return topy; }
	float getTerHeight(){ return TerHeight; }
	int getplatformLevel(){ return platformLevel; }
	int getlevels(){ return levels; }

	void hideRP(){ Rplatform.hide(); }
	void hideLP(){ Lplatform.hide(); }
	bool IsRPvisible(){ return Rplatform.isVisible(); }
	bool IsLPvisible(){ return Lplatform.isVisible(); }
	void resetPlatforms(){
		this->Rplatform.show();
		this->Lplatform.show();


		this->Lplatform.setplatform(this->Lplatform_x, this->Lplatform_y);
		this->Rplatform.setplatform(this->Rplatform_x, this->Rplatform_y);
	}
	void tilesToGreen(){
		YellowTilesNum = 0;
		for (int i = 0; i < levels; ++i){
			int size = piramidTilesCoords[i].size();
			for (int j = 0; j < size; ++j)
				piramidTilesCoords[i][j].setcolor(green);
		}

	}
};


class QBertFilm{
private:
	ALLEGRO_BITMAP* QBert;
	float width;
	float height;
public:
	QBertFilm(){
		QBert = al_load_bitmap("QBert.png");
		if (!QBert){
			std::cerr << "QBert.png not found or failed to load\n";
			exit(1);
		}
		width = al_get_bitmap_width(QBert);
		height = al_get_bitmap_height(QBert);
	}
	ALLEGRO_BITMAP* getQBert(){
		return this->QBert;
	}
	float getWidth(){ return this->width; }
	float getHeight(){ return this->height; }
	~QBertFilm(){
		al_destroy_bitmap(QBert);
	}
};

class SpeechFilm{
	ALLEGRO_BITMAP* Speech;
	float width;
	float height;
public:
	SpeechFilm(){
		Speech = al_load_bitmap("Speech.png");
		if (!Speech){
			std::cerr << "Speech.png not found or failed to load\n";
			exit(1);
		}
		width = al_get_bitmap_width(Speech);
		height = al_get_bitmap_height(Speech);
	}
	ALLEGRO_BITMAP* getSpeech(){
		return this->Speech;
	}
	float getWidth(){ return this->width; }
	float getHeight(){ return this->height; }
	~SpeechFilm(){
		al_destroy_bitmap(Speech);
	}
};
enum QBstate{
	onAir, standing, fall
};

class QBert{
private:
	QBertFilm qbert;
	SpeechFilm speech;
	speed sp;
	int sprites_num;
	float spriteWidth;
	float spriteHeight;
	int level, pos;
	Dir dir;
	float x, y;
	QBstate state;
	observeQBert *observer;
protected:
	void drawImg(float x, float y){
		al_draw_bitmap(qbert.getQBert(), x, y, NULL);
	}
	void drawRUat(float x, float y){
		al_draw_bitmap_region(qbert.getQBert(), 0, 0, spriteWidth, qbert.getHeight(), x, y, NULL);
	}
	void drawJRUat(float x, float y){
		al_draw_bitmap_region(qbert.getQBert(), spriteWidth, 0, spriteWidth, spriteHeight, x, y, NULL);
	}
	void drawLUat(float x, float y){
		al_draw_bitmap_region(qbert.getQBert(), 2 * spriteWidth, 0, spriteWidth, spriteHeight, x, y, NULL);
	}
	void drawJLUat(float x, float y){
		al_draw_bitmap_region(qbert.getQBert(), 3 * spriteWidth, 0, spriteWidth, spriteHeight, x, y, NULL);
	}
	void drawRDat(float x, float y){
		al_draw_bitmap_region(qbert.getQBert(), 4 * spriteWidth, 0, spriteWidth, spriteHeight, x, y, NULL);
	}
	void drawJRDat(float x, float y){
		al_draw_bitmap_region(qbert.getQBert(), 5 * spriteWidth, 0, spriteWidth, spriteHeight, x, y, NULL);
	}
	void drawLDat(float x, float y){
		al_draw_bitmap_region(qbert.getQBert(), 6 * spriteWidth, 0, spriteWidth, spriteHeight, x, y, NULL);
	}
	void drawJLDat(float x, float y){
		al_draw_bitmap_region(qbert.getQBert(), 7 * spriteWidth, 0, spriteWidth, spriteHeight, x, y, NULL);
	}

	void drawRU(){
		al_draw_bitmap_region(qbert.getQBert(), 0, 0, spriteWidth, qbert.getHeight(), x, y, NULL);
	}
	void drawJRU(){
		al_draw_bitmap_region(qbert.getQBert(), spriteWidth, 0, spriteWidth, spriteHeight, x, y, NULL);
	}
	void drawLU(){
		al_draw_bitmap_region(qbert.getQBert(), 2 * spriteWidth, 0, spriteWidth, spriteHeight, x, y, NULL);
	}
	void drawJLU(){
		al_draw_bitmap_region(qbert.getQBert(), 3 * spriteWidth, 0, spriteWidth, spriteHeight, x, y, NULL);
	}
	void drawRD(){
		al_draw_bitmap_region(qbert.getQBert(), 4 * spriteWidth, 0, spriteWidth, spriteHeight, x, y, NULL);
	}
	void drawJRD(){
		al_draw_bitmap_region(qbert.getQBert(), 5 * spriteWidth, 0, spriteWidth, spriteHeight, x, y, NULL);
	}
	void drawLD(){
		al_draw_bitmap_region(qbert.getQBert(), 6 * spriteWidth, 0, spriteWidth, spriteHeight, x, y, NULL);
	}
	void drawJLD(){
		al_draw_bitmap_region(qbert.getQBert(), 7 * spriteWidth, 0, spriteWidth, spriteHeight, x, y, NULL);
	}

public:
	QBert(controler* cnt, int sprites_num){
		this->sprites_num = sprites_num;
		this->spriteWidth = qbert.getWidth() / sprites_num;
		this->spriteHeight = qbert.getHeight();
		al_convert_mask_to_alpha(qbert.getQBert(), al_map_rgb(255, 255, 255));
		al_convert_mask_to_alpha(speech.getSpeech(), al_map_rgb(0, 255, 255));
		observer = (observeQBert*)cnt;
		observer->observeQB(sprites_num, spriteWidth, spriteHeight);
	}

	bool InitQBert(int level, int pos, speed sp, Dir dir){
		if (level >= control.getterrain_levels() || pos >= control.getQbLevelpositions(level).size())
			return false;
		this->level = level;
		this->pos = pos;
		this->dir = dir;
		this->sp = sp;
		this->x = control.getQbposition_x(level, pos);
		this->y = control.getQbposition_y(level, pos);
		this->state = standing;
		return true;
	}
	void reset(){
		level = pos = 0;
		x = control.getQbposition_x(level, pos);
		y = control.getQbposition_y(level, pos);
		dir = RU;
		state = standing;
	}


	void drawLives(int lives, float x, float y){
		for (int i = 0; i < lives; ++i){
			al_draw_bitmap_region(qbert.getQBert(), spriteWidth, 0, spriteWidth, spriteHeight, x, y + i*(spriteHeight + 5), NULL);
		}
	}

	Dir getDir(){ return dir; }
	float getx(){ return x; }
	float gety(){ return y; }
	float getbbx1(){ return x + 8; }
	float getbby1(){ return y + 8; }
	float getbbx2(){ return x + 20; }
	float getbby2(){ return y + 24; }

	int getlevel(){ return level; }
	int getpos(){ return pos; }
	float getWidth(){ return spriteWidth; }
	float getHeight(){ return spriteHeight; }
	speed getspeed(){ return sp; }
	QBstate getstate(){ return state; }

	void setDir(const Dir &dir){ this->dir = dir; }
	void setxy(const float &x, const float &y){
		this->x = x;
		this->y = y;
	}
	void sety(const float &y){ this->y = y; }
	void setlevel(const int &level){
		this->level = level;
	}
	void setpos(const int &pos){
		this->pos = pos;
	}
	void setstate(QBstate state){ this->state = state; }
	void drawSpeech(){
		al_draw_bitmap_region(speech.getSpeech(), 0, 0, speech.getWidth(), speech.getHeight(), x, y - spriteHeight, NULL);
	}
	void QBdraw(){
		if (getDir() == RU){
			if (getstate() == standing)
				drawRUat(getx(), gety());
			else
				drawJRUat(getx(), gety());
		}
		else if (getDir() == LD){
			if (getstate() == standing)
				drawLDat(getx(), gety());
			else
				drawJLDat(getx(), gety());
		}
		else if (getDir() == RD){
			if (getstate() == standing)
				drawRDat(getx(), gety());
			else
				drawJRDat(getx(), gety());
		}
		else if (getDir() == LU){
			if (getstate() == standing)
				drawLUat(getx(), gety());
			else
				drawJLUat(getx(), gety());
		}
	}

};


int findDir(float sx, float sy, float dx, float dy){
	if (sx < sy && dx < dy)
		return RU;
	else if (sx < dx && sy > dy)
		return RD;
	else if (sx > dx && sy > dy)
		return LD;
	else if (sx > dx && sy < dy)
		return LU;
	return -1;
}


class balloonFilms{
private:
	ALLEGRO_BITMAP* redBallfilm, *purpleBallfilm;
	float RBallwidth, RBallheight, PBallwidth, PBallheight;
public:
	balloonFilms(){
		redBallfilm = al_load_bitmap("redball.png");
		purpleBallfilm = al_load_bitmap("purpleball.png");
		if (!redBallfilm || !purpleBallfilm){
			std::cerr << "redball.png or purpleball.png not found or failed to load\n";
			exit(1);
		}
		RBallwidth = al_get_bitmap_width(redBallfilm);
		RBallheight = al_get_bitmap_height(redBallfilm);
		PBallwidth = al_get_bitmap_width(purpleBallfilm);
		PBallheight = al_get_bitmap_height(purpleBallfilm);

	}
	~balloonFilms(){
		al_destroy_bitmap(redBallfilm);
		al_destroy_bitmap(purpleBallfilm);
	}
	ALLEGRO_BITMAP* getRBallfilm(){
		return this->redBallfilm;
	}
	ALLEGRO_BITMAP* getPBallfilm(){
		return this->purpleBallfilm;
	}
	float getRBfilmWidth(){ return this->RBallwidth; }
	float getRBfilmHeight(){ return this->RBallheight; }
	float getPBfilmWidth(){ return this->PBallwidth; }
	float getPBfilmHeight(){ return this->PBallheight; }


};
enum ballState{
	ballOnGround, ballReadyToJump, ballOnAir, ballfalling
};
class ball{
private:
	bool isVisible;
	int level, pos;
	ballState state;
	float x, y;
	bool Red;
	Dir dir;
	delay_q sd, vd;
public:
	ball() :isVisible(true){ sd(15); vd(60); }
	ball(int level, int pos, float x, float y, ballState state, bool isRed, Dir dir) :isVisible(true){
		this->level = level;
		this->pos = pos;
		this->state = state;
		this->x = x;
		this->y = y;
		this->Red = isRed;
		this->dir = dir;
		sd(15); vd(60);
	}
	void setball(int level, int pos, float x, float y, ballState state, bool isRed, Dir dir){
		this->level = level;
		this->pos = pos;
		this->state = state;
		this->x = x;
		this->y = y;
		this->Red = isRed;
		this->dir = dir;
	}
	void setlevel(int level){ this->level = level; }
	void setpos(int pos){ this->pos = pos; }
	void setDir(Dir dir){ this->dir = dir; }
	void setx(float x){ this->x = x; }
	void sety(float y){ this->y = y; }
	void setstate(ballState state){ this->state = state; }
	void setstateSlowly(ballState state){
		if (sd()){
		}
		else{
			this->state = state;
			sd(15);
		}
	}
	bool setisVisibleSlowly(bool v){
		if (vd()){
		}
		else{
			this->isVisible = v;
			vd(60);
			return true;
		}
		return false;
	}

	ballState getState(){ return this->state; }
	float getx(){ return x; }
	float gety(){ return y; }
	int getlevel(){ return level; }
	int getpos(){ return pos; }
	bool isRed(){ return Red; }
	Dir getDir(){ return dir; }
	bool getisVisible(){ return isVisible; }
};


class SnakeFilm{
private:
	ALLEGRO_BITMAP* snakefilm;
	float snakewidth, snakeheight;
public:
	SnakeFilm(){
		snakefilm = al_load_bitmap("snake.png");
		if (!snakefilm){
			std::cerr << "snake.png not found or failed to load\n";
			exit(1);
		}
		snakewidth = al_get_bitmap_width(snakefilm);
		snakeheight = al_get_bitmap_height(snakefilm);
		snakewidth = al_get_bitmap_width(snakefilm);
		snakeheight = al_get_bitmap_height(snakefilm);

	}
	~SnakeFilm(){
		al_destroy_bitmap(snakefilm);
	}
	ALLEGRO_BITMAP* getsnakefilm(){
		return this->snakefilm;
	}

	float getsnakeWidth(){ return this->snakewidth; }
	float getsnakeHeight(){ return this->snakeheight; }

};

class snake{
	SnakeFilm sn;
	float x, y;
	int level, pos;
	Dir dir;
	speed sp;
	float spriteWidth, spriteHeight;
	bool Standing;
protected:
	void drawS(){
		if (dir == RU){
			al_draw_bitmap_region(sn.getsnakefilm(), 0, 0, spriteWidth, spriteHeight, x, y, 0);
		}
		else if (dir == LU){
			al_draw_bitmap_region(sn.getsnakefilm(), 2 * spriteWidth, 0, spriteWidth, spriteHeight, x, y, 0);
		}
		else if (dir == RD){
			al_draw_bitmap_region(sn.getsnakefilm(), 4 * spriteWidth, 0, spriteWidth, spriteHeight, x, y, 0);
		}
		else if (dir == LD){
			al_draw_bitmap_region(sn.getsnakefilm(), 6 * spriteWidth, 0, spriteWidth, spriteHeight, x, y, 0);
		}
	}
	void drawJ(){
		if (dir == RU){
			al_draw_bitmap_region(sn.getsnakefilm(), 1 * spriteWidth, 0, spriteWidth, spriteHeight, x, y, 0);
		}
		else if (dir == LU){
			al_draw_bitmap_region(sn.getsnakefilm(), 3 * spriteWidth, 0, spriteWidth, spriteHeight, x, y, 0);
		}
		else if (dir == RD){
			al_draw_bitmap_region(sn.getsnakefilm(), 5 * spriteWidth, 0, spriteWidth, spriteHeight, x, y, 0);
		}
		else if (dir == LD){
			al_draw_bitmap_region(sn.getsnakefilm(), 7 * spriteWidth, 0, spriteWidth, spriteHeight, x, y, 0);
		}
	}
public:
	snake(int level, int pos, float x, float y, speed sp) :level(level), pos(pos), x(x), y(y), sp(sp), Standing(true){
		al_convert_mask_to_alpha(sn.getsnakefilm(), al_map_rgb(255, 255, 255));
		spriteWidth = sn.getsnakeWidth() / 8;
		spriteHeight = sn.getsnakeHeight();
	}
	snake(int level, int pos, speed sp) :level(level), pos(pos), sp(sp), Standing(true){
		al_convert_mask_to_alpha(sn.getsnakefilm(), al_map_rgb(255, 255, 255));
		spriteWidth = sn.getsnakeWidth() / 8;
		spriteHeight = sn.getsnakeHeight();
	}
	void setlevel(int level){ this->level = level; }
	void setpos(int pos){ this->pos = pos; }
	void setDir(Dir dir){ this->dir = dir; }
	void setx(float x){ this->x = x; }
	void sety(float y){ this->y = y; }
	void setxy(float x, float y){
		this->x = x;
		this->y = y;
	}
	void setStanding(bool standing){ this->Standing = standing; }

	float getx(){ return x; }
	float gety(){ return y; }
	float getbbx1(){ return x; }
	float getbby1(){ return y + spriteHeight / 2 - 10; }
	float getbbx2(){ return x + spriteWidth; }
	float getbby2(){ return y + spriteHeight - 10; }
	float getbbWidth(){ return spriteWidth; }
	float getbbHeight(){ return spriteHeight / 2; }
	int getlevel(){ return level; }
	int getpos(){ return pos; }
	float getWidth(){ return spriteWidth; }
	float getHeight(){ return spriteHeight; }
	speed getspeed(){ return sp; }
	bool isStanding(){ return this->Standing; }

	void draw(){
		if (Standing == true){
			drawS();
		}
		else{
			drawJ();
		}
	}
};

class balloonEnemies{
private:
	balloonFilms balloonfilm;
	ball *fallingball;
	std::list<ball> balloons;
	speed sp;

	snake *sn;

	bool isPurpleValid;
public:
	balloonEnemies(speed sp) :sp(sp), isPurpleValid(true), fallingball(NULL), sn(NULL){
		al_convert_mask_to_alpha(balloonfilm.getRBallfilm(), al_map_rgb(0, 255, 255));
		al_convert_mask_to_alpha(balloonfilm.getPBallfilm(), al_map_rgb(0, 255, 255));
	}

	bool push(Dir d, bool isRed){
		if (!isRed && isPurpleValid){
			isPurpleValid = false;
		}
		else if (!isRed){
			return false;
		}
		ball b;
		if (d == LD){
			b.setball(-1, -1, control.getQbposition_x(1, 0), control.getQbposition_y(1, 0) - 50, ballfalling, isRed, d);
		}
		else if (d == RD){
			b.setball(-1, -1, control.getQbposition_x(1, 1), control.getQbposition_y(1, 1) - 50, ballfalling, isRed, d);
		}
		else{
			std::cout << "cannot push\n";
			return false;
		}
		balloons.push_back(b);
		fallingball = &balloons.back();
		return true;
	}
	void drawBalloon(ball b){
		if (!b.getisVisible()){
			return;
		}
		if (b.isRed()){
			if (b.getState() == ballOnGround)
				al_draw_bitmap_region(balloonfilm.getRBallfilm(), 2 * balloonfilm.getRBfilmWidth() / 3, 0, balloonfilm.getRBfilmWidth() / 3, balloonfilm.getRBfilmHeight(), b.getx(), b.gety(), NULL);
			else if (b.getState() == ballOnAir || b.getState() == ballfalling){
				al_draw_bitmap_region(balloonfilm.getRBallfilm(), 0, 0, balloonfilm.getRBfilmWidth() / 3, balloonfilm.getRBfilmHeight(), b.getx(), b.gety(), NULL);
			}
			else if (b.getState() == ballReadyToJump){
				al_draw_bitmap_region(balloonfilm.getRBallfilm(), balloonfilm.getRBfilmWidth() / 3, 0, balloonfilm.getRBfilmWidth() / 3, balloonfilm.getRBfilmHeight(), b.getx(), b.gety(), NULL);
			}
		}
		else{
			if (b.getState() == ballOnGround)
				al_draw_bitmap_region(balloonfilm.getPBallfilm(), 2 * balloonfilm.getPBfilmWidth() / 3, 0, balloonfilm.getPBfilmWidth() / 3, balloonfilm.getPBfilmHeight(), b.getx(), b.gety(), NULL);
			else if (b.getState() == ballOnAir || b.getState() == ballfalling){
				al_draw_bitmap_region(balloonfilm.getPBallfilm(), 0, 0, balloonfilm.getPBfilmWidth() / 3, balloonfilm.getPBfilmHeight(), b.getx(), b.gety(), NULL);
			}
			else if (b.getState() == ballReadyToJump){
				al_draw_bitmap_region(balloonfilm.getPBallfilm(), balloonfilm.getPBfilmWidth() / 3, 0, balloonfilm.getPBfilmWidth() / 3, balloonfilm.getPBfilmHeight(), b.getx(), b.gety(), NULL);
			}
		}
	}
	void drawBalloons(){
		for (std::list<ball>::iterator it = balloons.begin(); it != balloons.end(); ++it)
			drawBalloon(*it);
	}
	ball *getFallingBall(){ return fallingball; }
	void setFallingBall(ball* b){ fallingball = b; }
	speed getspeed(){ return sp; }
	ball* getBalloonAt(int i){
		if (i < balloons.size()){
			std::list<ball>::iterator tmp = balloons.begin();
			advance(tmp, i);
			return &(*tmp);
		}
		return NULL;
	}
	float getbbx1(int i){
		if (i < balloons.size()){
			std::list<ball>::iterator tmp = balloons.begin();
			advance(tmp, i);
			return tmp->getx() + 10;
		}
		return -1;
	}
	float getbbx2(int i){
		if (i < balloons.size()){
			std::list<ball>::iterator tmp = balloons.begin();
			advance(tmp, i);
			return tmp->getx() + 30;
		}
		return -1;
	}
	float getbby1(int i){
		if (i < balloons.size()){
			std::list<ball>::iterator tmp = balloons.begin();
			advance(tmp, i);
			return tmp->gety() + 10;
		}
		return -1;
	}
	float getbby2(int i){
		if (i < balloons.size()){
			std::list<ball>::iterator tmp = balloons.begin();
			advance(tmp, i);
			return tmp->gety() + 25;
		}
		return -1;
	}
	int getSize(){ return balloons.size(); };

	void removeball(int index){
		std::list<ball>::iterator tmp = balloons.begin();
		advance(tmp, index);
		if (!tmp->isRed()){
			sn = new snake(tmp->getlevel(), tmp->getpos(), control.getSnakeposition_x(tmp->getlevel(), tmp->getpos()), control.getSnakeposition_y(tmp->getlevel(), tmp->getpos()), sp);
		}
		balloons.erase(tmp);
	}
	void removeSnake(){
		isPurpleValid = true;
		if (sn){
			delete sn;
			sn = NULL;
		}
	}
	void reset(){
		removeSnake();
		if (fallingball){
			fallingball = NULL;
		}
		balloons.clear();
	}

	bool existSnake(){
		if (sn == NULL)
			return false;
		return true;
	}
	snake* getSnake(){
		return sn;
	}
	snake** getSnake_ptr(){
		return &sn;
	}

};

class SnakeAnimator{
	balloonEnemies *enemies;
	bool running;
	delay_q delay;
	float QBx, QBy;
public:
	SnakeAnimator(balloonEnemies* enemies) : enemies(enemies), running(false) {
		delay(15);
	}
	void start(float qbx, float qby){
		assert(enemies->getSnake(), "there is not snake\n");
		running = true;
		QBx = qbx;
		QBy = qby;

		Dir dir = follow();
		if (dir != NONE)
			enemies->getSnake()->setDir(dir);

		enemies->getSnake()->setStanding(false);
	}
	bool isRunning(){ return running; }
	bool Delay(){ return delay(); }
	void jump(Dir dir){
		assert(enemies->getSnake(), "there is not snake\n");
		if (dir == RU){
			enemies->getSnake()->setxy(enemies->getSnake()->getx() + control.getl() * enemies->getSnake()->getspeed(), enemies->getSnake()->gety() - enemies->getSnake()->getspeed());
			if (enemies->getSnake()->gety() < control.getSnakeposition_y(enemies->getSnake()->getlevel() - 1, enemies->getSnake()->getpos())){
				running = false;
				enemies->getSnake()->setlevel(enemies->getSnake()->getlevel() - 1);
				enemies->getSnake()->setxy(control.getSnakeposition_x(enemies->getSnake()->getlevel(), enemies->getSnake()->getpos()), control.getSnakeposition_y(enemies->getSnake()->getlevel(), enemies->getSnake()->getpos()));
				enemies->getSnake()->setStanding(true);
				delay(15);
			}
		}
		else if (dir == LD){
			enemies->getSnake()->setxy(enemies->getSnake()->getx() - control.getl() * enemies->getSnake()->getspeed(), enemies->getSnake()->gety() + enemies->getSnake()->getspeed());
			if (enemies->getSnake()->gety() > control.getSnakeposition_y(enemies->getSnake()->getlevel() + 1, enemies->getSnake()->getpos())){
				running = false;
				enemies->getSnake()->setlevel(enemies->getSnake()->getlevel() + 1);
				enemies->getSnake()->setxy(control.getSnakeposition_x(enemies->getSnake()->getlevel(), enemies->getSnake()->getpos()), control.getSnakeposition_y(enemies->getSnake()->getlevel(), enemies->getSnake()->getpos()));
				enemies->getSnake()->setStanding(true);
				delay(15);
			}
		}
		else if (dir == RD){
			enemies->getSnake()->setxy(enemies->getSnake()->getx() + control.getl() * enemies->getSnake()->getspeed(), enemies->getSnake()->gety() + enemies->getSnake()->getspeed());
			if (enemies->getSnake()->gety() > control.getSnakeposition_y(enemies->getSnake()->getlevel() + 1, enemies->getSnake()->getpos())){
				running = false;
				enemies->getSnake()->setlevel(enemies->getSnake()->getlevel() + 1);
				enemies->getSnake()->setpos(enemies->getSnake()->getpos() + 1);
				enemies->getSnake()->setxy(control.getSnakeposition_x(enemies->getSnake()->getlevel(), enemies->getSnake()->getpos()), control.getSnakeposition_y(enemies->getSnake()->getlevel(), enemies->getSnake()->getpos()));
				enemies->getSnake()->setStanding(true);
				delay(15);
			}
		}
		else if (dir == LU){
			enemies->getSnake()->setxy(enemies->getSnake()->getx() - control.getl() * enemies->getSnake()->getspeed(), enemies->getSnake()->gety() - enemies->getSnake()->getspeed());
			if (enemies->getSnake()->gety() < control.getSnakeposition_y(enemies->getSnake()->getlevel() - 1, enemies->getSnake()->getpos() - 1)){
				running = false;
				enemies->getSnake()->setlevel(enemies->getSnake()->getlevel() - 1);
				enemies->getSnake()->setpos(enemies->getSnake()->getpos() - 1);
				enemies->getSnake()->setxy(control.getSnakeposition_x(enemies->getSnake()->getlevel(), enemies->getSnake()->getpos()), control.getSnakeposition_y(enemies->getSnake()->getlevel(), enemies->getSnake()->getpos()));
				enemies->getSnake()->setStanding(true);
				delay(15);
			}
		}
		else if (dir == NONE){
			running = false;
			enemies->getSnake()->setStanding(true);
			delay(15);
		}
	}
	bool wasQBfound(){
		int index = enemies->getSnake()->getpos();
		if (QBx == control.getTile(enemies->getSnake()->getlevel(), enemies->getSnake()->getpos())->getpost_x() && QBy == control.getTile(enemies->getSnake()->getlevel(), enemies->getSnake()->getpos())->getpost_y()){
			return true;
		}
		return false;
	}
	Dir follow(){
		int index = enemies->getSnake()->getpos();
		float sx = control.getTilesOfLevel(enemies->getSnake()->getlevel())[index].getpost_x();
		float sy = control.getTilesOfLevel(enemies->getSnake()->getlevel())[index].getpost_y();
		if (QBx < sx){
			if (QBy < sy){
				return LU;
			}
			else if (QBy>sy){
				return LD;
			}
			else{
				return LU;
			}
		}
		else if (QBx>sx){
			if (QBy < sy){
				return RU;
			}
			else if (QBy>sy){
				return RD;
			}
			else{
				return RU;
			}
		}
		else{
			if (QBy < sy){
				return RU; // or LU
			}
			else if (QBy>sy){
				return LD; // or RD
			}
			else{
				return NONE;//qbert position == snake position
			}
		}

	}
	void reset(){
		running = false;
		delay(15);
	}
};

class balljumpAnimator{
	balloonEnemies *balloons;
	Terrain * ter;
	bool running;
	delay_q delay;
protected:
	void jump(ball &b, int index){
		if (b.getDir() == LD){
			b.setx(b.getx() - control.getl()*balloons->getspeed());
			b.sety(b.gety() + balloons->getspeed());
			if (b.getlevel() == control.getterrain_levels() - 1){
				if (b.gety()>ter->getTopy() + ter->getTerHeight()){
					balloons->removeball(index);
				}
			}
			else if (b.gety() > control.getQbposition_y(b.getlevel() + 1, b.getpos())){
				running = false;
				b.setlevel(b.getlevel() + 1);
				b.setx(control.getQbposition_x(b.getlevel(), b.getpos()));
				b.sety(control.getQbposition_y(b.getlevel(), b.getpos()));
				b.setstate(ballOnGround);
			}
		}
		else if (b.getDir() == RD){
			b.setx(b.getx() + control.getl() * balloons->getspeed());
			b.sety(b.gety() + balloons->getspeed());
			if (b.getlevel() == control.getterrain_levels() - 1){
				if (b.gety()>ter->getTopy() + ter->getTerHeight()){
					balloons->removeball(index);
				}
			}
			else if (b.gety() > control.getQbposition_y(b.getlevel() + 1, b.getpos())){
				running = false;
				b.setlevel(b.getlevel() + 1);
				b.setpos(b.getpos() + 1);
				b.setx(control.getQbposition_x(b.getlevel(), b.getpos()));
				b.sety(control.getQbposition_y(b.getlevel(), b.getpos()));
				b.setstate(ballOnGround);
			}
		}

	}
public:
	balljumpAnimator(balloonEnemies *balloons, Terrain* ter) :balloons(balloons), ter(ter), running(false) {
		delay(15);
	}
	bool isRunning(){ return running; }
	void setRunning(bool r){ running = r; }
	bool Delay(){ return delay(); }
	void setdelay(){ delay(15); }
	void start_jump(){
		running = true;
		for (int i = 0; i < balloons->getSize(); ++i){
			if (balloons->getBalloonAt(i)->getState() == ballOnGround){
				//start(*balloons->getBalloonAt(i));
				balloons->getBalloonAt(i)->setstateSlowly(ballOnAir);
				balloons->getBalloonAt(i)->setDir(RandomDownDir());
			}
		}
	}

	void jump(){
		for (int i = 0; i < balloons->getSize(); ++i){
			if (balloons->getBalloonAt(i)->getState() == ballOnAir){
				jump(*balloons->getBalloonAt(i), i);
			}
		}
	}

	void reset(){
		running = false;
		delay(15);
	}
};
class ballfallingOnAnimator{
	balloonEnemies *balloons;
	bool running;
	delay_q delay;
public:
	ballfallingOnAnimator(balloonEnemies *balloons) :balloons(balloons), running(false) {
		delay(15);
	}
	bool isRunning(){ return running; }
	void setRunning(bool r){ running = r; }
	bool Delay(){ return delay(); }
	void start(){
		running = true;
	}
	void falling(ball &b){
		if (b.getState() == ballfalling){
			b.sety(b.gety() + GRAVITY);
			if (b.gety()>control.getQbposition_y(1, 0)){
				if (b.getDir() == LD){
					b.setball(1, 0, control.getQbposition_x(1, 0), control.getQbposition_y(1, 0), ballOnGround, b.isRed(), b.getDir());
					balloons->setFallingBall(NULL);
					running = false;
				}
				else{
					b.setball(1, 1, control.getQbposition_x(1, 1), control.getQbposition_y(1, 1), ballOnGround, b.isRed(), b.getDir());
					balloons->setFallingBall(NULL);
					running = false;
				}
			}
		}
	}
	void resest(){
		running = false;
		delay(15);
	}
};
class platformToTopAnimator{
	Terrain *terrain;
	QBert *qbert;
	bool runningRP;
	bool runningLP;
	delay_q delay;
public:
	platformToTopAnimator(Terrain *terrain, QBert* qb) :terrain(terrain), qbert(qb), runningRP(false), runningLP(false){
		delay(30);
	}
	bool isRunningRP(){ return runningRP; }
	void setRunningRP(bool r){ runningRP = r; }
	bool isRunningLP(){ return runningLP; }
	void setRunningLP(bool r){ runningLP = r; }
	bool Delay(){ return delay(); }
	void startRP(){
		runningRP = true;
		qbert->setstate(standing);
	}
	void startLP(){
		runningLP = true;
		qbert->setstate(standing);
	}
	bool runRp(){
		std::vector<std::array<float, 2>> tmp = control.getRplatformPath();
		int size = tmp.size();
		static int cur = 0;
		bool ret = true;
		terrain->setRplatform(tmp[cur][0], tmp[cur][1]);
		qbert->setxy(tmp[cur][0] + terrain->getPlatformWidth() / 4, tmp[cur][1] - 13);
		terrain->drawRplatform();
		if (cur + 1 != size){
			++cur;
		}
		else{
			if (!delay()){
				delay(30);
			}
			else{
				runningRP = false;
				qbert->setstate(standing);
				qbert->setlevel(0);
				qbert->setpos(0);
				qbert->setxy(control.getQbposition_x(0, 0), control.getQbposition_y(0, 0));
				if (terrain->getTileColor(0, 0) == green){
					terrain->setTileColor(yellow, 0, 0);
					terrain->YellowTilesIncrement();
					ret = false;
				}
				terrain->hideRP();
				cur = 0;
			}
		}
		return ret;
	}

	bool runLp(){
		std::vector<std::array<float, 2>> tmp = control.getLplatformPath();
		int size = tmp.size();
		static int cur = 0;
		bool ret = true;
		terrain->setLplatform(tmp[cur][0], tmp[cur][1]);
		qbert->setxy(tmp[cur][0] + terrain->getPlatformWidth() / 4, tmp[cur][1] - 13);
		terrain->drawLplatform();
		if (cur + 1 != size){
			++cur;
		}
		else{
			if (!delay()){
				delay(30);
			}
			else{
				runningLP = false;
				qbert->setstate(standing);
				qbert->setlevel(0);
				qbert->setpos(0);
				qbert->setxy(control.getQbposition_x(0, 0), control.getQbposition_y(0, 0));
				if (terrain->getTileColor(0, 0) == green){
					terrain->setTileColor(yellow, 0, 0);
					terrain->YellowTilesIncrement();
					ret = false;
				}
				terrain->hideLP();
				cur = 0;
			}
		}
		return ret;
	}
	void reset(){
		runningRP = false;
		runningLP = false;
		delay(30);
	}
};

class QBjumpAnimator{
	Terrain *terrain;
	QBert *qbert;
	bool running;
	delay_q delay;
public:
	QBjumpAnimator(Terrain *terrain, QBert* qb) :terrain(terrain), qbert(qb), running(false){
		delay(5);
	}
	bool isRunning(){ return running; }
	void setRunning(bool r){ running = r; }
	bool Delay(){ return delay(); }
	void start(){
		running = true;
		qbert->setstate(onAir);
	}
	bool jump(Dir dir){
		if (dir == RU){
			qbert->setxy(qbert->getx() + control.getl() * qbert->getspeed(), qbert->gety() - qbert->getspeed());
			if (qbert->gety() < control.getQbposition_y(qbert->getlevel() - 1, qbert->getpos())){
				running = false;
				qbert->setlevel(qbert->getlevel() - 1);
				qbert->setxy(control.getQbposition_x(qbert->getlevel(), qbert->getpos()), control.getQbposition_y(qbert->getlevel(), qbert->getpos()));
				qbert->setstate(standing);
				if (terrain->getTileColor(qbert->getlevel(), qbert->getpos()) == green){
					terrain->setTileColor(yellow, qbert->getlevel(), qbert->getpos());
					terrain->YellowTilesIncrement();
					delay(5);
					return false;
				}
				delay(5);
			}
		}
		else if (dir == LD){
			qbert->setxy(qbert->getx() - control.getl() * qbert->getspeed(), qbert->gety() + qbert->getspeed());
			if (qbert->gety() > control.getQbposition_y(qbert->getlevel() + 1, qbert->getpos())){
				running = false;
				qbert->setlevel(qbert->getlevel() + 1);
				qbert->setxy(control.getQbposition_x(qbert->getlevel(), qbert->getpos()), control.getQbposition_y(qbert->getlevel(), qbert->getpos()));
				qbert->setstate(standing);
				if (terrain->getTileColor(qbert->getlevel(), qbert->getpos()) == green){
					terrain->setTileColor(yellow, qbert->getlevel(), qbert->getpos());
					terrain->YellowTilesIncrement();
					delay(5);
					return false;
				}
				delay(5);
			}
		}
		else if (dir == RD){
			qbert->setxy(qbert->getx() + control.getl() * qbert->getspeed(), qbert->gety() + qbert->getspeed());
			if (qbert->gety() > control.getQbposition_y(qbert->getlevel() + 1, qbert->getpos())){
				running = false;
				qbert->setlevel(qbert->getlevel() + 1);
				qbert->setpos(qbert->getpos() + 1);
				qbert->setxy(control.getQbposition_x(qbert->getlevel(), qbert->getpos()), control.getQbposition_y(qbert->getlevel(), qbert->getpos()));
				qbert->setstate(standing);
				if (terrain->getTileColor(qbert->getlevel(), qbert->getpos()) == green){
					terrain->setTileColor(yellow, qbert->getlevel(), qbert->getpos());
					terrain->YellowTilesIncrement();
					delay(5);
					return false;
				}
				delay(5);
			}
		}
		else if (dir == LU){
			qbert->setxy(qbert->getx() - control.getl() * qbert->getspeed(), qbert->gety() - qbert->getspeed());
			if (qbert->gety() < control.getQbposition_y(qbert->getlevel() - 1, qbert->getpos() - 1)){
				running = false;
				qbert->setlevel(qbert->getlevel() - 1);
				qbert->setpos(qbert->getpos() - 1);
				qbert->setxy(control.getQbposition_x(qbert->getlevel(), qbert->getpos()), control.getQbposition_y(qbert->getlevel(), qbert->getpos()));
				qbert->setstate(standing);
				if (terrain->getTileColor(qbert->getlevel(), qbert->getpos()) == green){
					terrain->setTileColor(yellow, qbert->getlevel(), qbert->getpos());
					terrain->YellowTilesIncrement();
					delay(5);
					return false;
				}
				delay(5);
			}
		}
		return true;
	}
	void reset(){
		running = false;
		delay(5);
	}
};

class QBFallsOutAnimator{
	QBert *qbert;
	bool running;
	delay_q delay;
	Dir dir;
	float destx;
	float desty;

	bool  falling = false;
public:
	QBFallsOutAnimator(QBert* qb) : qbert(qb), running(false){
		delay(5);
	}
	bool isRunning(){ return running; }
	void setRunning(bool r){ running = r; }
	void start(Dir dir){
		running = true;
		qbert->setstate(QBstate::fall);
		this->dir = dir;
		if (dir == RU){
			destx = qbert->getx() + 32;
			desty = qbert->gety() - 25;
		}
		else if (dir == RD){
			destx = qbert->getx() + 32;
			desty = qbert->gety() + 25;
		}
		else if (dir == LU){
			destx = qbert->getx() - 32;
			desty = qbert->gety() - 25;
		}
		else if (dir == LD){
			destx = qbert->getx() - 32;
			desty = qbert->gety() + 25;
		}
	}
	void jumpOut(){
		if (dir == RU){
			if (qbert->getx() > destx && qbert->gety() < desty){
				falling = true;
				return;
			}
			if (!falling)
				qbert->setxy(qbert->getx() + control.getl() * qbert->getspeed(), qbert->gety() - qbert->getspeed());
		}
		else if (dir == RD){
			if (qbert->getx() > destx && qbert->gety() > desty){
				falling = true;
				return;
			}
			if (!falling)
				qbert->setxy(qbert->getx() + control.getl() * qbert->getspeed(), qbert->gety() + qbert->getspeed());
		}
		else if (dir == LU){
			if (qbert->getx() < destx && qbert->gety() < desty){
				falling = true;
				return;
			}
			if (!falling)
				qbert->setxy(qbert->getx() - control.getl() * qbert->getspeed(), qbert->gety() - qbert->getspeed());
		}
		else if (dir == LD){
			if (qbert->getx() < destx && qbert->gety() > desty){
				falling = true;
				return;
			}
			if (!falling)
				qbert->setxy(qbert->getx() - control.getl() * qbert->getspeed(), qbert->gety() + qbert->getspeed());
		}
	}
	void fall(){
		jumpOut();
		if (falling){
			qbert->sety(qbert->gety() + 7);
		}
	}
	void reset(){
		running = false;
		falling = false;
		delay(5);
	}
};

class SnakeOutAnimator{
	snake **sn;
	bool running;
	delay_q delay;
	Dir dir;
	float destx;
	float desty;

	bool  falling = false;
public:
	SnakeOutAnimator(snake** sn) : sn(sn), running(false){
		delay(5);
	}
	void reset(){
		running = false;
		falling = false;
		delay(5);
		destx = desty = 0;
	}
	bool isRunning(){ return running; }
	void setRunning(bool r){ running = r; }
	void start(Dir dir){
		running = true;
		this->dir = dir;
		if (dir == RU){
			destx = (*sn)->getx() + 32;
			desty = (*sn)->gety() - 25;
		}
		else if (dir == LU){
			destx = (*sn)->getx() - 32;
			desty = (*sn)->gety() - 25;
		}
	}
	void jumpOut(){
		if (dir == RU){
			if ((*sn)->getx() > destx && (*sn)->gety() < desty){
				falling = true;
				return;
			}
			if (!falling)
				(*sn)->setxy((*sn)->getx() + control.getl() * (*sn)->getspeed(), (*sn)->gety() - (*sn)->getspeed());
		}

		else if (dir == LU){
			if ((*sn)->getx() < destx && (*sn)->gety() < desty){
				falling = true;
				return;
			}
			if (!falling)
				(*sn)->setxy((*sn)->getx() - control.getl() * (*sn)->getspeed(), (*sn)->gety() - (*sn)->getspeed());
		}

	}

	void fall(){
		jumpOut();
		if (falling){
			(*sn)->sety((*sn)->gety() + 7);
		}
	}

};

class Lives{
	int lives;
public:
	Lives(int lives) :lives(lives){}
	void decrement(){ --lives; }
	void increment(){ ++lives; }
	int getLives(){ return lives; }
};
class game_qb{
	short level;
	Score score;
	int MaxScore;
	Lives lives;
	short int difficulty;

	ALLEGRO_DISPLAY *display;
	ALLEGRO_FONT *font12;
	ALLEGRO_FONT *font36;
	ALLEGRO_FONT *font48;
	ALLEGRO_SAMPLE* winSoundEffect, *fallingSoundEffect, *failSoundEffect, *HitsoundSoundEffect;
	ALLEGRO_KEYBOARD_STATE keyState;
	ALLEGRO_TIMER *timer;
	ALLEGRO_EVENT_QUEUE *event_queue;

	Terrain *ter;
	QBert *qbert;
	balloonEnemies *enemies;

	platformToTopAnimator *platform_anim;
	SnakeAnimator *snake_jump;
	ballfallingOnAnimator *ballFallingOnAnim;
	balljumpAnimator *enemiesJump;
	QBjumpAnimator *qb_jump;
	QBFallsOutAnimator *qb_fall;
	SnakeOutAnimator *snake_fall;
	//enemies->getSnake()->getbbx1(), enemies->getSnake()->getbby1()
	bool detectSnakeCollision(){
		if (!enemies->existSnake() || qbert->getbbx2() < enemies->getSnake()->getbbx1() || qbert->getbbx1() > enemies->getSnake()->getbbx2() || qbert->getbby1() > enemies->getSnake()->getbby2() || qbert->getbby2()<enemies->getSnake()->getbby1()){
			return false;
		}
		return true;
	}
	bool detectBalloonCollision(){
		for (int i = 0; i < enemies->getSize(); ++i){
			if (qbert->getbbx2() < enemies->getbbx1(i) || qbert->getbbx1() > enemies->getbbx2(i) || qbert->getbby1() > enemies->getbby2(i) || qbert->getbby2()<enemies->getbby1(i)){
			}
			else{
				return true;
			}
		}
		return false;
	}
	void raiseDifficulty(){
		if (difficulty>100)
			difficulty -= 50;
	}
protected:
	int loadScore(const char* filename){
		std::ifstream openfile(filename);
		if (openfile.is_open()){
			std::string score;
			std::getline(openfile, score);
			if (score.size() == 0)
				return 0;
			return std::stoi(score);
		}
		return -1;

	}
	void WriteScore(const char* filename){
		std::ofstream scorefile;
		scorefile.open(filename);
		if (scorefile.is_open()){
			scorefile << score.getSScore();
			scorefile.close();
		}
	}
public:
	game_qb( ) :  lives(3), difficulty(300){}
	bool initialize(){
		this->level = 0;
		this->MaxScore = loadScore("MaxScore");
		if (!al_init()){
			std::cerr << "failed to initialize allegro!\n";
			return false;
		}

		//al_set_new_display_flags(ALLEGRO_WINDOWED | ALLEGRO_RESIZABLE);
		display = al_create_display(ScreenWidth, ScreenHeight);
		if (!display){
			std::cerr << "failed to create display\n";
			return false;
		}
		al_set_window_position(display, 200, 100);
		al_set_window_title(display, "QBERT");


		al_init_primitives_addon();

		al_install_keyboard();
		al_install_audio();
		al_init_acodec_addon();
		al_init_image_addon();
		al_init_font_addon();
		al_init_ttf_addon();
		al_install_mouse();

		ter = new Terrain(7, &control);
		qbert = new QBert(&control, 8);

		control.initializeControler();
		qbert->InitQBert(0, 0, 5, RU);
		enemies = new balloonEnemies(3);

		platform_anim = new platformToTopAnimator(ter, qbert);
		snake_jump = new SnakeAnimator(enemies);
		ballFallingOnAnim = new ballfallingOnAnimator(enemies);
		enemiesJump = new balljumpAnimator(enemies, ter);
		qb_jump = new QBjumpAnimator(ter, qbert);
		qb_fall = new QBFallsOutAnimator(qbert);
		snake_fall = new SnakeOutAnimator(enemies->getSnake_ptr());

		font12 = al_load_font("arial.ttf", 12, 0);
		font36 = al_load_font("arial.ttf", 36, 0);
		font48 = al_load_font("arial.ttf", 48, 0);
		if (!font12 || !font36 || !font48){
			std::cerr << "failed to load font\n";
			return false;
		}

		winSoundEffect = al_load_sample("WinSoundEffect.wav");
		failSoundEffect = al_load_sample("FailSoundEffect.wav");
		fallingSoundEffect = al_load_sample("FallingSoundEffect.wav");
		HitsoundSoundEffect = al_load_sample("HitsoundSoundEffect.wav");
		if (!winSoundEffect || !failSoundEffect || !fallingSoundEffect || !HitsoundSoundEffect){
			std::cerr << "failed to load sounds\n";
			return false;
		}
		al_reserve_samples(2);

		timer = al_create_timer(1.0 / FPS); //if !timer ...
		if (!timer) {
			std::cerr << "failed to create timer!\n";
			return false;
		}
		event_queue = al_create_event_queue();
		if (!event_queue) {
			std::cerr << "failed to create event_queue!\n";
			//al_destroy_bitmap(bouncer);
			//al_destroy_display(display);
			//al_destroy_timer(timer);
			return false;
		}

		al_register_event_source(event_queue, al_get_timer_event_source(timer));
		al_register_event_source(event_queue, al_get_display_event_source(display));
		al_register_event_source(event_queue, al_get_keyboard_event_source());

		return true;
	}
	/*void menu(){
		bool repeat = true;
		ALLEGRO_EVENT events;
		al_wait_for_event(event_queue, &events);
		while (repeat){
			if (events.type == ALLEGRO_EVENT_KEY_UP){
				repeat = false;
			}
			al_draw_text(font12, al_map_rgb(255, 0, 255), 50, 50, NULL, "PRESS \"S\" TO START GAME");
			al_clear_to_color(al_map_rgb(0, 0, 0));
			al_flip_display();
		}
	}*/
	bool start(){
		al_flush_event_queue(event_queue);
		bool repeat = true;
		bool active = false;
		bool ret = false, win = false;
		bool runningAnim = false;
		bool collision = false;
		bool immortal = false;
		delay_q pause(0);
		bool draw = true;
		Dir dir = RD;
		Dir balldir;
		bool Speak = false;
		al_start_timer(timer);
		while (repeat){
			ALLEGRO_EVENT events;
			al_wait_for_event(event_queue, &events);
			al_get_keyboard_state(&keyState);
			if (events.type == ALLEGRO_EVENT_TIMER){
				if (!pause()){
					draw = true;
					collision = false;
					Speak = false;
					if (ret)
						return win;
					if (lives.getLives() <= 0){
						pause(180);
						ret = true;
						win = false;
					}
					active = true;
					if (!qb_jump->isRunning() && !platform_anim->isRunningLP() && !platform_anim->isRunningRP() && !qb_fall->isRunning()){
						if (qb_jump->Delay()){
						}
						else if (al_key_down(&keyState, ALLEGRO_KEY_DOWN)){
							dir = LD;
							if (qbert->getlevel() + 1 < control.getterrain_levels()){
								qb_jump->start();
							}
							else{
								qb_fall->start(dir);
								al_play_sample(fallingSoundEffect, 1.0, 0.0, 2.0, ALLEGRO_PLAYMODE_ONCE, 0);
							}
						}
						else if (al_key_down(&keyState, ALLEGRO_KEY_UP)){
							dir = RU;
							if (qbert->getpos() < qbert->getlevel()){
								qb_jump->start();
							}
							else if (qbert->getlevel() == ter->getplatformLevel() && ter->IsRPvisible()){ //5 is the level of platform
								platform_anim->startRP();
							}
							else{
								qb_fall->start(dir);
								al_play_sample(fallingSoundEffect, 1.0, 0.0, 2.0, ALLEGRO_PLAYMODE_ONCE, 0);
							}
						}
						else if (al_key_down(&keyState, ALLEGRO_KEY_RIGHT)){
							dir = RD;
							if (qbert->getlevel() + 1 < control.getterrain_levels()){
								qb_jump->start();
							}
							else{
								qb_fall->start(dir);
								al_play_sample(fallingSoundEffect, 1.0, 0.0, 2.0, ALLEGRO_PLAYMODE_ONCE, 0);
							}

						}
						else if (al_key_down(&keyState, ALLEGRO_KEY_LEFT)){
							dir = LU;
							if (qbert->getpos() - 1 >= 0){
								qb_jump->start();
							}
							else if (qbert->getlevel() == ter->getplatformLevel() && ter->IsLPvisible()){ //5 is the level of platform
								platform_anim->startLP();
							}
							else{
								qb_fall->start(dir);
								al_play_sample(fallingSoundEffect, 1.0, 0.0, 2.0, ALLEGRO_PLAYMODE_ONCE, 0);
							}
						}
						else if (al_key_down(&keyState, ALLEGRO_KEY_SPACE)){
							pause(10);
							immortal = !immortal;
						}
						else{
							active = false;
						}
						if (active){
							qbert->setDir(dir);
						}
					}
					if (qb_jump->isRunning()){
						if (qb_jump->jump(dir)){
						}
						else{
							score.increase(25);
						}
					}
					else if (platform_anim->isRunningRP()){
						if (platform_anim->runRp()){
						}
						else{
							score.increase(25);
						}
					}
					else if (platform_anim->isRunningLP()){
						if (platform_anim->runLp()){
						}
						else{
							score.increase(25);
						}
					}
					else if (qb_fall->isRunning()){
						qb_fall->fall();
						if (qbert->gety() > ScreenHeight + 350){
							if (!immortal)
								lives.decrement();
							enemies->reset();
							ballFallingOnAnim->resest();
							enemiesJump->reset();
							snake_jump->reset();
							qbert->reset();
							qb_jump->reset();
							qb_fall->reset();
							pause(90);
							draw = false;
						}
					}
					balldir = RandomDownDir();

					if (rand() % difficulty == 0 && !ballFallingOnAnim->isRunning()){
						if (enemies->push(balldir, true))
							ballFallingOnAnim->start();
					}
					if (rand() % 300 == 0 && !ballFallingOnAnim->isRunning()){
						if (enemies->push(balldir, false))
							ballFallingOnAnim->start();
					}
					if (ballFallingOnAnim->isRunning()){
						ballFallingOnAnim->falling(*enemies->getFallingBall());
					}
					enemiesJump->start_jump();
					enemiesJump->jump();
					snake* sn = enemies->getSnake();
					if (sn != NULL){
						if (!snake_jump->isRunning() && !snake_fall->isRunning()){
							if (platform_anim->isRunningLP() || platform_anim->isRunningRP()){
								if (sn->getlevel() == ter->getplatformLevel()){
									if (sn->getpos() == 0){
										snake_fall->start(LU);
									}
									else if (sn->getpos() == ter->getplatformLevel()){
										snake_fall->start(RU);
									}
								}
							}
						}
						else if (snake_fall->isRunning()){
							snake_fall->fall();
							if (sn->gety() > ScreenHeight - 70){
								snake_fall->reset();
								enemies->removeSnake();
							}
						}
						if (!snake_jump->isRunning() && !snake_fall->isRunning() && sn != NULL){
							if (snake_jump->Delay()){}
							else{
								snake_jump->start(control.getTilesOfLevel(qbert->getlevel())[qbert->getpos()].getpost_x(), control.getTilesOfLevel(qbert->getlevel())[qbert->getpos()].getpost_y());
							}
						}
						else if (snake_jump->isRunning()){
							Dir dir = snake_jump->follow();
							snake_jump->jump(dir);
						}
					}

					if (ter->getYellowTilesNum() == control.gettiles_num()){
						pause(180);
						ret = true;
						win = true;
					}


					if (((detectSnakeCollision() || detectBalloonCollision())) && !qb_fall->isRunning()){
						collision = true;
						pause(90);
						al_play_sample(HitsoundSoundEffect, 1.0, 0.0, 2.0, ALLEGRO_PLAYMODE_ONCE, 0);
						if (!immortal){
							lives.decrement();
						}
						else{
							draw = false;
						}
						Speak = true;
						enemies->reset();
						ballFallingOnAnim->resest();
						enemiesJump->reset();
						snake_jump->reset();
					}


					if (draw && al_is_event_queue_empty(event_queue)){
						bool TerBehind = false;
						if (qb_fall->isRunning()){
							if (qbert->getlevel() == ter->getlevels() - 1 && (qbert->getDir() == LD || qbert->getDir() == RD)){
								TerBehind = true;
							}
							else{
								qbert->QBdraw();
								if (immortal){
									al_draw_filled_rectangle(qbert->getbbx1(), qbert->getbby1(), qbert->getbbx2(), qbert->getbby2(), al_map_rgba_f(.6, 0, .6, .6));
								}
							}
						}

						if (snake_fall->isRunning()){
							if (enemies->existSnake()){
								enemies->getSnake()->draw();
								if (immortal)
									al_draw_filled_rectangle(enemies->getSnake()->getbbx1(), enemies->getSnake()->getbby1(), enemies->getSnake()->getbbx2(), enemies->getSnake()->getbby2(), al_map_rgba_f(.6, 0, .6, .6));
							}
						}

						ter->drawPiramide();
						ter->drawRplatform();
						ter->drawLplatform();

						/*for (int i = 0; i < control.getterrain_levels(); ++i){
						std::vector<tile> tmp = control.getTilesOfLevel(i);
						int size = tmp.size();
						for (int j = 0; j < size; ++j)
						al_draw_filled_circle(tmp[j].getpost_x(), tmp[j].getpost_y(), 5, al_map_rgba_f(.6, 0, .6, .6));
						}*/
						//ter->showTop();
						if (!qb_fall->isRunning() || TerBehind){
							qbert->QBdraw();
							if (immortal){
								al_draw_filled_rectangle(qbert->getbbx1(), qbert->getbby1(), qbert->getbbx2(), qbert->getbby2(), al_map_rgba_f(.6, 0, .6, .6));
							}
						}

						if (!snake_fall->isRunning()){
							if (enemies->existSnake()){
								enemies->getSnake()->draw();
								if (immortal)
									al_draw_filled_rectangle(enemies->getSnake()->getbbx1(), enemies->getSnake()->getbby1(), enemies->getSnake()->getbbx2(), enemies->getSnake()->getbby2(), al_map_rgba_f(.6, 0, .6, .6));
							}
						}

						qbert->drawLives(lives.getLives(), ScreenWidth / 2 + 475, 200);
						if (Speak)
							qbert->drawSpeech();
						enemies->drawBalloons();
						if (immortal){
							for (int i = 0; i < enemies->getSize(); ++i){
								al_draw_filled_rectangle(enemies->getbbx1(i), enemies->getbby1(i), enemies->getbbx2(i), enemies->getbby2(i), al_map_rgba_f(.6, 0, .6, .6));

							}
						}

						if (immortal){
							if (collision){
								al_draw_text(font36, al_map_rgb(128, 0, 0), ScreenWidth / 2 - 50, ScreenHeight / 2, NULL, "COLLISION");
							}
						}

						al_draw_text(font12, al_map_rgb(255, 0, 255), 50, 50, NULL, "GIORGOS KRITIKAKIS 3309");
						al_draw_text(font12, al_map_rgb(255, 0, 255), 50, 70, NULL, "University of Crete");
						al_draw_text(font12, al_map_rgb(255, 0, 255), 50, 90, NULL, "Department of Computer  Science");
						al_draw_text(font12, al_map_rgb(255, 0, 255), 50, 110, NULL, "CS - 454. Development of Intelligent Interfaces and Games");
						al_draw_text(font12, al_map_rgb(255, 0, 255), 50, 130, NULL, "Term Project, Fall Semester 2017");

						al_draw_text(font36, al_map_rgb(255, 0, 255), ScreenWidth / 2 - 30, 50, NULL, std::string("LEVEL " + std::to_string(level)).c_str());
						al_draw_text(font36, al_map_rgb(255, 0, 255), ScreenWidth / 2 + 400, 50, NULL, std::string("Max score:" + std::to_string(MaxScore)).c_str());
						al_draw_text(font36, al_map_rgb(255, 0, 255), ScreenWidth / 2 + 475, 100, NULL, score.getSScore());


						al_flip_display();
						al_clear_to_color(al_map_rgb(0, 0, 0));


					}
				}
			}
			else if (events.type == ALLEGRO_EVENT_KEY_UP && events.keyboard.keycode == ALLEGRO_KEY_P){
				if (pause.isBlocked()){
					pause.unblock();
				}
				else{
					pause.block();
				}

			}
			else if (events.type == ALLEGRO_EVENT_DISPLAY_CLOSE){
				repeat = false;
			}
		}

		return false;
	}
	void end(){
		if (MaxScore < score.getIScore())
			WriteScore("MaxScore");

		al_destroy_sample(winSoundEffect);
		al_destroy_sample(failSoundEffect);
		al_destroy_sample(fallingSoundEffect);
		al_destroy_sample(HitsoundSoundEffect);
		al_destroy_timer(timer);
		al_destroy_display(display);
	}
	void nextLevel(){
		raiseDifficulty();
		if (level % 3 == 2)
			lives.increment();
		++level;
		enemies->reset();
		qbert->reset();
		ter->resetPlatforms();
		ter->tilesToGreen();
		qb_fall->reset();
		qb_jump->reset();
		snake_fall->reset();
		enemiesJump->reset();
		ballFallingOnAnim->resest();
		platform_anim->reset();
		snake_jump->reset();
	}
	
	void messageEnd(){
		al_clear_to_color(al_map_rgb(0, 0, 0));
		if (MaxScore < score.getIScore()){
			al_draw_text(font48, al_map_rgb(255, 0, 255), ScreenWidth / 2 - 150, ScreenHeight / 2 - 150, NULL, "CONGRATULATIONS");
			al_draw_text(font48, al_map_rgb(255, 0, 255), ScreenWidth / 2, ScreenHeight / 2 - 50, NULL, score.getSScore());
			al_play_sample(winSoundEffect, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
		}
		else{
			al_draw_text(font48, al_map_rgb(255, 0, 255), ScreenWidth / 2 - 100, ScreenHeight / 2 - 50, NULL, "YOU LOSE");
			al_play_sample(failSoundEffect, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
		}
		al_flip_display();
		al_rest(4.0);
	}
	void messageLevel(){
		al_draw_text(font48, al_map_rgb(255, 0, 255), ScreenWidth / 2 - 100, ScreenHeight / 2 - 50, NULL, std::string("LEVEL " + std::to_string(level)).c_str());
		al_flip_display();
		al_rest(5);
		//std::this_thread::sleep_for(std::chrono::seconds(2));
	}

	void menu(){
		int start_but_x1 = ScreenWidth / 2 - 300, start_but_y1 = ScreenHeight / 2 - 50;
		int start_but_x2 = ScreenWidth / 2 + 300, start_but_y2 = ScreenHeight / 2 + 20;
		bool repeat = true;
		ALLEGRO_EVENT_QUEUE *menu_event_queue = al_create_event_queue();
		al_register_event_source(menu_event_queue , al_get_keyboard_event_source());
		al_register_event_source(menu_event_queue, al_get_mouse_event_source());
		if (!menu_event_queue) {
			std::cerr << "failed to create menu_event_queue!\n";
			//al_destroy_bitmap(bouncer);
			//al_destroy_display(display);
			//al_destroy_timer(timer);
			return ;
		}
		while (repeat){
			ALLEGRO_EVENT event;
			al_draw_rectangle(start_but_x1, start_but_y1, start_but_x2, start_but_y2 , al_map_rgb(255, 0, 255), 9.0);
			al_draw_text(font48, al_map_rgb(255, 0, 255), ScreenWidth / 2 - 200, ScreenHeight / 2 - 50, NULL, "press here to start");
			al_flip_display();
			al_clear_to_color(al_map_rgb(0, 0, 0));

			al_wait_for_event(menu_event_queue, &event);

			if (event.type == ALLEGRO_EVENT_KEY_UP)
			{
				if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
					exit(0);
			}
			else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
			{
				int x = event.mouse.x, y = event.mouse.y;
				if (event.mouse.button & 1){
					if (event.mouse.x >= start_but_x1 && x < start_but_x2 &&
						y >= start_but_y1 && y < start_but_y2)
					{
						repeat = false;
					}
				}
			}

		}

		al_stop_timer(timer);
		al_destroy_event_queue(menu_event_queue);
	}
};


int main(int argc, char** argv){
	game_qb game;
	if (game.initialize()){
		game.menu();
		while ( game.start() ){
			game.nextLevel();
		}
		game.messageEnd();
		game.end();
	}
	else{
		std::cout << "could not initialize game\n";
	}
}




