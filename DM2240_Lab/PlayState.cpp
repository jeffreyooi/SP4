#include "GameState.h"
#include "GameStateManager.h"
#include "PlayState.h"
#include "MenuState.h"
#include <mmsystem.h>
#include "glext.h"
#include <lua.hpp>

#pragma comment(linker, "/subsystem:\"console\" /entry:\"mainCRTStartup\"")
#pragma warning(disable:4996)

CPlayState CPlayState::thePlayState;
lua_State *L;

void CPlayState::Init(void)
{
	Cam = NULL;
	player = NULL;
	w = glutGet(GLUT_WINDOW_WIDTH);
	h = glutGet(GLUT_WINDOW_HEIGHT);
	state = 0;
	pos_x = 0;
	pos_y = 0;
	WX = w;
	WY = h;
	mapOffset_x = 0;
	mapOffset_y = 0;
	tileOffset_x = 0;
	tileOffset_y = 0;
	mapFineOffset_x = 0;
	mapFineOffset_y = 0;
	theNumOfTiles_Height = 0;
	theNumOfTiles_Width = 0;
	rearWallOffset_x = 0;
	rearWallOffset_y = 0;
	rearWalltileOffset_x = 0;
	rearWalltileOffset_y = 0;
	rearWallFineOffset_x = 0;
	rearWallFineOffset_y = 0;
	selection = 1;
	enemycounter = 0;
	ClearMapCounter = 0;
	ClearLaneCounter = 0;
	heroAnimationCounter = 0;
	progress = 1;
	info = 0;
	spawntimer = 0;
	ratio = 0;
	m_fps = 30;
	m_speed = 1;
	m_worldSizeY = 100;
	m_worldSizeX = m_worldSizeY;
	gamesave = 1;
	saveandload = false;
	playmusic = true;
	pause = false;
	soundon = true;
	upgrade = false;
	next = false;
	back = false;
	level = 1;
	powerMap = false;
	powerLane = false;
	powerfired = false;
	levelloaded = true;
	m_ghost = Tower::TOWER_NORMAL;
	m_ghost.SetActive(true);
	for (int i = 0; i < 255; i++)
	{
		myKeys[i] = false;
	}
	Math::InitRNG();
	mouseInfo.lastX = 0;
	mouseInfo.lastY = 0;

	Cam = new Camera(Camera::LAND_CAM);
	Cam->SetPosition(0.0, 2.0, -5.0);
	Cam->SetDirection(0.0, 0.0, 1.0);

	// Player init
	player = new PlayerInfo();

	se = createIrrKlangDevice();
	playSound();

	// Enemy progress init
	tEnemyProgress = new CEnemyProgress();
	tEnemyProgress->initEnemyCounter();
	tEnemyProgress->SetPosX(0);
	tEnemyProgress->SetPosY(0);

	// Initialization
	theMap = new CMap();
	theMap->Init(SCREEN_HEIGHT, SCREEN_WIDTH * 2, SCREEN_HEIGHT, SCREEN_WIDTH * 2, TILE_SIZE);
	theMap->LoadMap("bin/maps/MapDesign.csv");

	/*RenderBackground();
	RenderTileMap();
	SpawnTowers();
	SpawnEnemy();*/

	theNumOfTiles_Height = theMap->getNumOfTiles_ScreenHeight();
	theNumOfTiles_Width = theMap->getNumOfTiles_ScreenWidth();

	glEnable(GL_TEXTURE_2D);							// Enable Texture Mapping ( NEW )
	LoadTGA(&BackgroundTexture[0], "bin/textures/XPDefaultBackground.tga");
	LoadTGA(&TileMapTexture[0], "bin/textures/tile0_blank.tga");
	LoadTGA(&TileMapTexture[1], "bin/textures/t1.tga");
	LoadTGA(&TileMapTexture[2], "bin/textures/mouseover.tga");
	LoadTGA(&TileMapTexture[3], "bin/textures/mouseclick.tga");
	LoadTGA(&TileMapTexture[4], "bin/textures/t1.tga");

	LoadTGA(&Music[0], "bin/menu/Ingamemenu/set/tracklist.tga");
	LoadTGA(&TowerTexture[0], "bin/tower/Heavy.tga");
	LoadTGA(&TowerTexture[1], "bin/textures/cannontower.tga");
	LoadTGA(&TowerTexture[2], "bin/tower/Soldier.tga");
	LoadTGA(&TowerTexture[3], "bin/tower/Heavy.tga");

	LoadTGA(&Icon[0], "bin/tower/Heavy.tga");
	LoadTGA(&Icon[1], "bin/tower/tower2.tga");
	LoadTGA(&Icon[2], "bin/tower/Heavy.tga");
	LoadTGA(&Icon[3], "bin/tower/Soldier.tga");

	LoadTGA(&Quit[0], "bin/exit/Savegame.TGA");
	LoadTGA(&Quit[1], "bin/exit/Savegameyes.TGA");
	LoadTGA(&Quit[2], "bin/exit/Savegameno.TGA");

	LoadTGA(&BulletTexture[0], "bin/textures/normalbullet.tga");
	LoadTGA(&BulletTexture[1], "bin/textures/cannonbullet.tga");
	LoadTGA(&BulletTexture[2], "bin/textures/slowbullet.tga");
	LoadTGA(&BulletTexture[3], "bin/textures/lightningbullet.tga");

	LoadTGA(&CreepTexture[0], "bin/textures/redpoker.tga");
	LoadTGA(&CreepTexture[1], "bin/textures/redcard.tga");
	LoadTGA(&CreepTexture[2], "bin/textures/ahlong.tga");

	LoadTGA(&Upgrade[0], "bin/textures/upgrade.tga");
	LoadTGA(&Heart[0], "bin/textures/heart.tga");

	LoadTGA(&Story[0], "bin/textures/textBox.tga");
	LoadTGA(&Power[0], "bin/textures/powerMap.tga");
	LoadTGA(&Power[1], "bin/textures/powerLane.tga");
	LoadTGA(&PowerBoom[0], "bin/textures/clearMap.tga");
	LoadTGA(&PowerBoom[1], "bin/textures/clearLane.tga");
	// Load the attributes through text file
	LoadAtt();
	loadlevel();
	LoadSpawn();
}

void CPlayState::Cleanup()
{
	if (Cam != NULL)
	{
		delete Cam;
		Cam = NULL;
	}

	if (player != NULL)
	{
		delete player;
		player = NULL;
	}

	while (bulletList.size() > 0)
	{
		Bullet *bullet = bulletList.back();
		delete bullet;
		bulletList.pop_back();
	}

	while (enemyList.size() > 0)
	{
		Enemy *creep = enemyList.back();
		delete creep;
		enemyList.pop_back();
	}

	while (towerList.size() > 0)
	{
		Tower *tower = towerList.back();
		delete tower;
		towerList.pop_back();
	}

	while (towerClone.size() > 0)
	{
		Tower *clone = towerClone.back();
		delete clone;
		towerClone.pop_back();
	}

	while (enemyClone.size() > 0)
	{
		Enemy *clone = enemyClone.back();
		delete clone;
		enemyClone.pop_back();
	}

	while (spawnList.size() > 0)
	{
		Spawn *spawn = spawnList.back();
		delete spawn;
		spawnList.pop_back();
	}

	while (powerList.size() > 0)
	{
		Powerup *power = powerList.back();
		delete power;
		powerList.pop_back();
	}
}

void CPlayState::Pause()
{
}

void CPlayState::Resume()
{
}

void CPlayState::HandleEvents(CGameStateManager* theGSM)
{
	/*int m_iUserChoice = -1;

	do {
	cout << "CPlayState: Choose one <0> Go to Menu State : " ;
	cin >> m_iUserChoice;
	cin.get();

	switch (m_iUserChoice) {
	case 0:
	theGSM->ChangeState( CMenuState::Instance() );
	break;
	default:
	cout << "Invalid choice!\n";
	m_iUserChoice = -1;
	break;
	}
	} while (m_iUserChoice == -1);


	}*/

}

void CPlayState::Update(CGameStateManager* theGSM)
{
	static int frame = 0;
	static int lastTime = glutGet(GLUT_ELAPSED_TIME);
	++frame;
	int time = glutGet(GLUT_ELAPSED_TIME);
	float dt = ((time - lastTime) / 1000.f);
	lastTime = time;

	static int lastFPSTime = glutGet(GLUT_ELAPSED_TIME);
	if ((time - lastFPSTime) > 1000)
	{
		m_fps = (frame * 1000.f / (time - lastFPSTime));
		lastFPSTime = time;
		frame = 0;
	}

	w = glutGet(GLUT_WINDOW_WIDTH);
	h = glutGet(GLUT_WINDOW_HEIGHT);

	// timer for enemy to spawn
	spawntimer += dt;

	// Spawn enemies
	UpdateSpawn();

	// Deactivate out of bounds objects
	for (std::vector<Bullet *>::iterator it = bulletList.begin(); it != bulletList.end(); ++it)
	{
		Bullet *bullet = *it;
		if (bullet->GetPos().x >= SCREEN_WIDTH || bullet->GetPos().x < 0)
		{
			bullet->SetActive(false);
		}
	}

	for (std::vector<Enemy *>::iterator it = enemyList.begin(); it != enemyList.end(); ++it)
	{
		Enemy *creep = *it;
		if (creep->GetActive() == true)
		{
			if (creep->GetFire() == false)
			{
				// Monster moving speed
				creep->SetPos(Vector3(creep->GetPos().x + creep->GetVel().x * dt, creep->GetPos().y, 0));
				if (creep->GetPos().x <= 0)
				{
					creep->SetActive(false);
					tEnemyProgress->SetEnemyCounter(tEnemyProgress->GetEnemyCounter() - 1);
					enemycounter--;
					break;
				}
			}
		}
	}

	// Collision updates and unit triggers
	Update(dt);

}

void CPlayState::Draw(CGameStateManager* theGSM) {
	//int w = glutGet(GLUT_WINDOW_WIDTH);
	//int h = glutGet(GLUT_WINDOW_HEIGHT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();

	Cam->Update();

	// HUD
	Cam->SetHUD(true);

	RenderBackground();

	RenderTileMap();

	tower1();
	tower2();
	tower3();
	tower4();
	power1();
	power2();

	// Render towers
	for (std::vector<Tower *>::iterator it = towerList.begin(); it != towerList.end(); ++it)
	{
		Tower *tower = *it;
		if (tower->GetActive() == true)
		{
			DrawTower(tower);
		}
	}

	// Render enemies
	for (std::vector<Enemy *>::iterator it2 = enemyList.begin(); it2 != enemyList.end(); ++it2)
	{
		Enemy *creep = *it2;
		if (creep->GetActive() == true)
		{
			DrawEnemy(creep);
		}
	}

	// Render bullets
	for (std::vector<Bullet *>::iterator it3 = bulletList.begin(); it3 != bulletList.end(); ++it3)
	{
		Bullet *bullet = *it3;
		if (bullet->GetActive() == true)
		{
			DrawBullet(bullet);
		}
	}

	// Draw ghost cursor
	if (m_ghost.GetActive())
	{
		DrawTower(&m_ghost);
	}

	// Render update cursor
	if (upgrade == true)
	{
		RenderUpgrade(mouseInfo.lastX, (h - mouseInfo.lastY));
	}

	// Mouse over tower selection for info
	if (info > 0)
	{
		RenderInfo(mouseInfo.lastX, (h - mouseInfo.lastY));
	}

	// Draw powerup
	if (powerfired == true)
	{
		if (powerMap == true && ClearMapCounter > 0)
		{
			powerTex(true, 1);
		}
		else if (powerLane == true && ClearLaneCounter > 0)
		{
			powerTex(true, 2);
		}
	}

	tEnemyProgress->DrawEnemyCounter(500, 630); // Enemy Progress Bar
	DrawHeart();

	//// Pause == true render pause menu
	//if (Menu->GetMpausemenu() == true)
	//{
	//	ingamemenu();
	//}

	//// Pause Menu Settings
	//if (Menu->GetIGMsettings() == true)
	//{
	//	IGMSettings();
	//}

	// Track selection
	if (Tracklist == true)
	{
		track();
	}

	// On screen texts
	glColor3f(1.0f, 1.0f, 1.0f);
	char temp[512];

	sprintf_s(temp, "Gold: %d", player->GetGold());
	RenderStringOnScreen(0, 90, temp);
	/*sprintf_s(temp, "FPS: %.2f", m_fps);
	RenderStringOnScreen(0, 30, temp);*/
	sprintf_s(temp, "x %d", player->GetLife());
	RenderStringOnScreen(50, 60, temp);
	sprintf_s(temp, "Selection: %d", selection);
	RenderStringOnScreen(120, 90, temp);
	sprintf_s(temp, "Enemy left: %d", tEnemyProgress->GetEnemyCounter());
	RenderStringOnScreen(500, 600, temp);
	sprintf_s(temp, "x %d", ClearLaneCounter);
	RenderStringOnScreen(900, 80, temp);
	sprintf_s(temp, "x %d", ClearMapCounter);
	RenderStringOnScreen(800, 80, temp);

	Cam->SetHUD(false);

	glFlush();
	glutSwapBuffers();
	glutPostRedisplay();
}

void CPlayState::changeSize(int w, int h) {

	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	//if (h == 0)
	//	h = 1;

	//ratio = (float)(1.0f* w / h);

	//// Reset the coordinate system before modifying
	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();

	//// Set the viewport to be the entire window
	//glViewport(0, 0, w, h);

	//// Set the correct perspective.
	//gluPerspective(45, ratio, 1, 1000);
	//glMatrixMode(GL_MODELVIEW);

	if (h == 0)
		h = 1;

	ratio = (float)w / h;
	m_worldSizeX = m_worldSizeY * ratio;

	WX = w;
	WY = h;

	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, m_worldSizeX, 0, m_worldSizeY);
	//gluPerspective(45, ratio, 1, 1000);
	glMatrixMode(GL_MODELVIEW);
}

void CPlayState::inputKey(int key, int x, int y) {

	switch (key) {
	case GLUT_KEY_LEFT:
		break;
	case GLUT_KEY_RIGHT:
		break;
	case GLUT_KEY_UP:
		break;
	case GLUT_KEY_DOWN:
		break;
	}
}

void CPlayState::KeyboardDown(unsigned char key, int x, int y){

	myKeys[key] = true;
	switch (key)
	{
	case '1':
		selection = 1;
		break;
	case '2':
		selection = 2;
		break;
	case '3':
		selection = 3;
		break;
	case '4':
		selection = 4;
		break;
	case '5':
		if (ClearMapCounter > 0)
		{
			if (Powerup *power = FetchPower())
			{
				power->type = Powerup::POWER_CLEAR;
				// Powerup Check

				power->SetType(power->type);

				for (std::vector<Enemy *>::iterator it2 = enemyList.begin(); it2 != enemyList.end(); ++it2)
				{
					Enemy *creep = *it2;
					if (creep->GetActive())
					{
						creep->SetHealth(creep->GetHealth() - power->GetDmg());
						if (creep->GetHealth() <= 0)
						{
							creep->SetActive(false);
							soundTypes(creep->type, true);
							tEnemyProgress->SetEnemyCounter(tEnemyProgress->GetEnemyCounter() - 1);
							enemycounter--;
							player->SetGold(player->GetGold() + rand() % 50 + 10);
						}
					}

				}
			}
			ClearMapCounter--;
		}
		break;
	case '6':
		if (ClearLaneCounter > 0)
		{
			if (Powerup *power = FetchPower())
			{
				power->type = Powerup::POWER_LANE;

				power->SetType(power->type);

				for (std::vector<Enemy *>::iterator it2 = enemyList.begin(); it2 != enemyList.end(); ++it2)
				{
					Enemy *creep = *it2;
					if (creep->GetActive())
					{
						int y = (float)mouseInfo.lastY / WY * 7;

						if ((creep->GetPos().y * 7 / SCREEN_HEIGHT) - 0.5f == y)
						{
							creep->SetHealth(creep->GetHealth() - power->GetDmg());
							if (creep->GetHealth() <= 0)
							{
								creep->SetActive(false);
								soundTypes(creep->type, true);
								tEnemyProgress->SetEnemyCounter(tEnemyProgress->GetEnemyCounter() - 1);
								enemycounter--;
								player->SetGold(player->GetGold() + rand() % 50 + 10);
							}
						}
					}
				}
			}
			ClearLaneCounter--;
		}
		break;
	case '7':
		if (Powerup *power = FetchPower())
		{
			power->type = Powerup::POWER_DOWN;
			power->SetType(power->type);
		}
		break;
	case '-':
	case '_':
		sound.decreaseVolume();
		break;
	case '=':
	case '+':
		sound.increaseVolume();
		break;
	case ' ':
		if (playmusic == true)
		{
			sound.pause();
			playmusic = false;
		}
		else
		{
			sound.resume();
			playmusic = true;
		}
		break;
	case 27:
		exit(0);
		break;
	}
}

void CPlayState::KeyboardUp(unsigned char key, int x, int y){

	myKeys[key] = false;
}

void CPlayState::MouseMove(int x, int y) {
	int tile_topleft_x = (int)floor((float)(mapOffset_x + pos_x) / TILE_SIZE);
	int tile_topleft_y = (int)floor((float)pos_y / TILE_SIZE);
	mouseInfo.lastX = x;
	mouseInfo.lastY = y;
	std::cout << x << " " << y << std::endl;
	int X = (int)((float)x / WX * 10);
	int Y = (int)((float)y / WY * 7);
	/*int w = glutGet(GLUT_WINDOW_WIDTH);
	int h = glutGet(GLUT_WINDOW_HEIGHT);*/
	//mouseInfo.mLButtonUp = !!state;

	moverlevel1(x, y);

	if (!pause)
	{
		/*if (theMap->theScreenMap[Y][X] != 3 && X >= 1 && X <= 10 && Y >= 1 && Y <= 5)
		{
			m_ghost.SetActive(true);
			m_ghost.type = static_cast<Tower::TOWER_TYPE>(selection);
			m_ghost.SetPos(Vector3((float)x / w * SCREEN_WIDTH, (float)y / h * SCREEN_HEIGHT, 0));
			upgrade = false;
		}
		else if (theMap->theScreenMap[Y][X] == 3 && X >= 1 && X <= 10 && Y >= 1 && Y <= 5)
		{
			m_ghost.SetActive(false);
			upgrade = true;
		}
		else
		{
			m_ghost.SetActive(false);
		}*/

		if (X == 3 && Y == 0)
		{
			info = 1;
			upgrade = false;
		}
		else if (X == 4 && Y == 0)
		{
			info = 2;
			upgrade = false;
		}
		else if (X == 5 && Y == 0)
		{
			info = 3;
			upgrade = false;
		}
		else if (X == 6 && Y == 0)
		{
			info = 4;
			upgrade = false;
		}
		else
		{
			info = 0;
		}
	}
}

void CPlayState::MouseClick(int button, int state, int x, int y) {
	switch (button) {

	case GLUT_LEFT_BUTTON:
		if (state == 0)
		{
			mouseInfo.mLButtonUp = false;
			mclicklevel1(x, y);
		}
		else
			mouseInfo.mLButtonUp = true;
		mouseInfo.lastX = x;
		mouseInfo.lastY = y;
		break;

	case GLUT_RIGHT_BUTTON:
		if (state == 0)
			mouseInfo.mRButtonUp = false;
		else
			mouseInfo.mRButtonUp = true;
		mouseInfo.lastX = x;
		mouseInfo.lastY = y;
		break;
	}
}

bool CPlayState::LoadTGA(TextureImage *texture, char *filename)			// Loads A TGA File Into Memory
{
	GLubyte		TGAheader[12] = { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 };	// Uncompressed TGA Header
	GLubyte		TGAcompare[12];								// Used To Compare TGA Header
	GLubyte		header[6];									// First 6 Useful Bytes From The Header
	GLuint		bytesPerPixel;								// Holds Number Of Bytes Per Pixel Used In The TGA File
	GLuint		imageSize;									// Used To Store The Image Size When Setting Aside Ram
	GLuint		temp;										// Temporary Variable
	GLuint		type = GL_RGBA;								// Set The Default GL Mode To RBGA (32 BPP)

	/*
	// NEW version for opening a file
	FILE *file = NULL;
	errno_t err;

	// Open for read (will fail if file "crt_fopen_s.c" does not exist)
	err  = fopen_s( &file, filename, "rb" );
	if( err == 0 )
	{
	cout << "The file " << filename << " was opened" << endl;
	}
	else
	{
	cout << "The file " << filename << " was UNABLE to be opened!" << endl;
	}
	*/

	FILE *file = fopen(filename, "rb");						// Open The TGA File

	if (file == NULL ||										// Does File Even Exist?
		fread(TGAcompare, 1, sizeof(TGAcompare), file) != sizeof(TGAcompare) ||	// Are There 12 Bytes To Read?
		memcmp(TGAheader, TGAcompare, sizeof(TGAheader)) != 0 ||	// Does The Header Match What We Want?
		fread(header, 1, sizeof(header), file) != sizeof(header))				// If So Read Next 6 Header Bytes
	{
		if (file == NULL)									// Did The File Even Exist? *Added Jim Strong*
			return false;									// Return False
		else
		{
			fclose(file);									// If Anything Failed, Close The File
			return false;									// Return False
		}
	}

	texture->width = header[1] * 256 + header[0];			// Determine The TGA Width	(highbyte*256+lowbyte)
	texture->height = header[3] * 256 + header[2];			// Determine The TGA Height	(highbyte*256+lowbyte)

	if (texture->width <= 0 ||								// Is The Width Less Than Or Equal To Zero
		texture->height <= 0 ||								// Is The Height Less Than Or Equal To Zero
		(header[4] != 24 && header[4] != 32))					// Is The TGA 24 or 32 Bit?
	{
		fclose(file);										// If Anything Failed, Close The File
		return false;										// Return False
	}

	texture->bpp = header[4];							// Grab The TGA's Bits Per Pixel (24 or 32)
	bytesPerPixel = texture->bpp / 8;						// Divide By 8 To Get The Bytes Per Pixel
	imageSize = texture->width*texture->height*bytesPerPixel;	// Calculate The Memory Required For The TGA Data

	texture->imageData = (GLubyte *)malloc(imageSize);		// Reserve Memory To Hold The TGA Data

	if (texture->imageData == NULL ||							// Does The Storage Memory Exist?
		fread(texture->imageData, 1, imageSize, file) != imageSize)	// Does The Image Size Match The Memory Reserved?
	{
		if (texture->imageData != NULL)						// Was Image Data Loaded
			free(texture->imageData);						// If So, Release The Image Data

		fclose(file);										// Close The File
		return false;										// Return False
	}

	for (GLuint i = 0; i<int(imageSize); i += bytesPerPixel)		// Loop Through The Image Data
	{														// Swaps The 1st And 3rd Bytes ('R'ed and 'B'lue)
		temp = texture->imageData[i];							// Temporarily Store The Value At Image Data 'i'
		texture->imageData[i] = texture->imageData[i + 2];	// Set The 1st Byte To The Value Of The 3rd Byte
		texture->imageData[i + 2] = temp;					// Set The 3rd Byte To The Value In 'temp' (1st Byte Value)
	}

	fclose(file);											// Close The File

	// Build A Texture From The Data
	glGenTextures(1, &texture[0].texID);					// Generate OpenGL texture IDs

	glBindTexture(GL_TEXTURE_2D, texture[0].texID);			// Bind Our Texture
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	// Linear Filtered
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	// Linear Filtered

	if (texture[0].bpp == 24)									// Was The TGA 24 Bits
	{
		type = GL_RGB;										// If So Set The 'type' To GL_RGB
	}

	glTexImage2D(GL_TEXTURE_2D, 0, type, texture[0].width, texture[0].height, 0, type, GL_UNSIGNED_BYTE, texture[0].imageData);

	return true;											// Texture Building Went Ok, Return True
}

void CPlayState::RenderTileMap(void) {

	mapFineOffset_x = mapOffset_x % TILE_SIZE;

	glPushMatrix();
	for (int i = 0; i < theNumOfTiles_Height; i++)
	{
		for (int k = 0; k < theNumOfTiles_Width + 1; k++)
		{
			// If we have reached the right side of the Map, then do not display the extra column of tiles.
			if ((tileOffset_x + k) >= theMap->getNumOfTiles_MapWidth())
				break;
			glPushMatrix();
			glTranslatef((GLfloat)(k * TILE_SIZE - mapFineOffset_x), (GLfloat)(i * TILE_SIZE), 0.0f);
			glEnable(GL_TEXTURE_2D);
			glEnable(GL_BLEND);
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glBindTexture(GL_TEXTURE_2D, TileMapTexture[theMap->theScreenMap[i][tileOffset_x + k]].texID);
			glBegin(GL_QUADS);
			glTexCoord2f(0, 1); glVertex2f(0, 0);
			glTexCoord2f(0, 0); glVertex2f(0, TILE_SIZE);
			glTexCoord2f(1, 0); glVertex2f(TILE_SIZE, TILE_SIZE);
			glTexCoord2f(1, 1); glVertex2f(TILE_SIZE, 0);
			glEnd();
			glDisable(GL_BLEND);
			glDisable(GL_TEXTURE_2D);
			glPopMatrix();
		}
	}
	glPopMatrix();
}

void CPlayState::moverlevel1(int x, int y)
{
	int a = 0; // row
	for (float i = 0.0f; i < 0.708335f; i += 0.141667f)
	{
		for (float u = 0.0f; u < 0.9f; u += 0.1f)
		{
			if ((x > WX * (0.09875f + u)) && (x < WX * (0.1975f + u)) &&
				(y > WY * (0.12666f + i)) && (y < WY * (0.2683333f + i)) && (theMap->theScreenMap[1 + a][1 + (u * 10)] == 1))
			{
				theMap->theScreenMap[(unsigned int)(1 + a)][(unsigned int)(1 + (u * 10))] = 2;
			}
			if (((x < WX * (0.09875f + u)) || (x > WX * (0.1975f + u)) ||
				(y < WY * (0.12666f + i)) || (y > WY * (0.2683333f + i))) && theMap->theScreenMap[1 + a][1 + (u * 10)] == 2)
			{
				theMap->theScreenMap[(unsigned int)(1 + a)][(unsigned int)(1 + (u * 10))] = 1;
			}

			if ((x > WX * (0.09875f + u)) && (x < WX * (0.1975f + u)) &&
				(y > WY * (0.12666f + i)) && (y < WY * (0.2683333f + i)) && (theMap->theScreenMap[1 + a][1 + (u * 10)] == 4))
			{
				theMap->theScreenMap[(unsigned int)(1 + a)][(unsigned int)(1 + (u * 10))] = 3;
			}
			if (((x < WX * (0.09875f + u)) || (x > WX * (0.1975f + u)) ||
				(y < WY * (0.12666f + i)) || (y > WY * (0.2683333f + i))) && theMap->theScreenMap[1 + a][1 + (u * 10)] == 3)
			{
				theMap->theScreenMap[(unsigned int)(1 + a)][(unsigned int)(1 + (u * 10))] = 4;
			}
		}
		a++;
	}
}

void CPlayState::mclicklevel1(int x, int y)
{
	int X = (int)((float)x / WX * 10);
	int Y = (int)((float)y / WY * 7);

	if (X == 3 && Y == 0)
	{
		selection = 1;
	}

	if (X == 4 && Y == 0)
	{
		selection = 2;
	}

	if (X == 5 && Y == 0)
	{
		selection = 3;
	}

	if (X == 6 && Y == 0)
	{
		selection = 4;
	}

	if (X == 8 && Y == 0)
	{
		powerMap = true;
		powerLane = false;
	}
	if (X == 9 && Y == 0)
	{
		powerLane = true;
		powerMap = false;
	}

	Tower *tower = NULL;
	if (pause == false)
	{
		if (theMap->theScreenMap[Y][X] == 2) // tile not occupied
		{
			if (tower = FetchTower())
			{
				tower->type = static_cast<Tower::TOWER_TYPE>(selection);
				tower->SetAtt(towerClone[selection - 1]->GetFireRate(), towerClone[selection - 1]->GetCost(),
					towerClone[selection - 1]->GetDamage(), towerClone[selection - 1]->GetRange(), towerClone[selection - 1]->GetHealth());
				if (player->GetGold() >= tower->GetCost())
				{
					tower->SetActive(true);
					tower->SetLevel(1);
					tower->SetPos(Vector3((float)((X + 0.5f) * TILE_SIZE), (float)((Y + 0.5f) * TILE_SIZE), 0));
					player->SetGold(player->GetGold() - tower->GetCost());
					theMap->theScreenMap[Y][X] = 3;
				}
				else
				{
					tower->SetActive(false);
				}
			}
		}
		else if (theMap->theScreenMap[Y][X] == 3) // tile occupied
		{
			for (unsigned int i = 0; i < towerList.size(); ++i)
			{
				if (towerList[i]->GetActive() && towerList[i]->GetPos().x == (X + 0.5f) * TILE_SIZE && towerList[i]->GetPos().y == (Y + 0.5f) * TILE_SIZE)
				{
					if (player->GetGold() >= towerList[i]->GetCost() + 25 && towerList[i]->GetLevel() < 3)
					{
						towerList[i]->Upgrade();
						player->SetGold(player->GetGold() - towerList[i]->GetCost());
					}
				}
			}
		}
	}
}

void CPlayState::Update(float dt)
{
	// Check if in range
	for (std::vector<Tower *>::iterator it = towerList.begin(); it != towerList.end(); ++it)
	{
		Tower *tower = *it;
		tower->SetFire(false);
		if (tower->GetActive() == true)
		{
			for (std::vector<Enemy *>::iterator it = enemyList.begin(); it != enemyList.end(); ++it)
			{
				Enemy *creep = *it;
				if (creep->GetActive() == true)
				{
					if (tower->GetPos().y == creep->GetPos().y && creep->GetPos().x - tower->GetPos().x < tower->GetRange() && creep->GetPos().x > tower->GetPos().x)
					{
						if (tower->GetFireCounter() > 0)
						{
							tower->SetFireCounter(tower->GetFireCounter() - dt);
							break;
						}
						else if (tower->GetFireCounter() <= 0)
						{
							tower->SetFire(true);
							tower->SetFireCounter(tower->GetFireRate());
							break;
						}
					}
				}
			}
		}
	}

	// Fire bullets
	for (std::vector<Tower *>::iterator it = towerList.begin(); it != towerList.end(); ++it)
	{
		Tower *tower = *it;
		if (tower->GetActive() == true && tower->GetFire() == true)
		{
			Bullet *bullet;
			if (bullet = FetchBullet())
			{
				bullet->SetActive(true);
				//bullet->type = static_cast<Bullet::BULLET_TYPE>(tower->type);
				if (tower->type == Tower::TOWER_NORMAL)
				{
					bullet->type = Bullet::GO_NORMALBULLET;
				}
				else if (tower->type == Tower::TOWER_CANNON)
				{
					bullet->type = Bullet::GO_CANNONBULLET;
				}
				else if (tower->type == Tower::TOWER_SLOW)
				{
					bullet->type = Bullet::GO_SLOWBULLET;
				}
				else if (tower->type == Tower::TOWER_SHOCK)
				{
					bullet->type = Bullet::GO_SHOCKBULLET;
					bullet->SetHealth(3);
				}
				bullet->SetDamage(tower->GetDamage());
				bullet->SetVel(Vector3(200, 0, 0));
				bullet->SetPos(Vector3(tower->GetPos().x, tower->GetPos().y, 0));
			}
		}
	}

	//update bullet position
	for (unsigned int i = 0; i < bulletList.size(); ++i)
	{
		if (bulletList[i]->GetActive() && bulletList[i]->type != Bullet::GO_BOMBBULLET)
		{
			bulletList[i]->SetPos(Vector3(bulletList[i]->GetPos().x + bulletList[i]->GetVel().x * dt, bulletList[i]->GetPos().y, 0));
		}
	}

	// Bomb radius
	for (std::vector<Bullet *>::iterator itp = bulletList.begin(); itp != bulletList.end(); ++itp)
	{
		Bullet *go2 = *itp;
		if (go2->GetActive())
		{
			if (go2->type == Bullet::GO_BOMBBULLET)
			{
				//go2->scale.Set(10, 10, 0);
				for (std::vector<Enemy *>::iterator itp = enemyList.begin(); itp != enemyList.end(); ++itp)
				{
					Enemy *creepz = *itp;
					if (creepz->GetActive())
					{
						Vector3 temp3;
						temp3.x = creepz->GetPos().x;
						temp3.y = creepz->GetPos().y;
						if (abs((temp3 - go2->GetPos()).Length()) < 100)
						{
							creepz->SetHealth(creepz->GetHealth() - go2->GetDamage());
							go2->SetActive(false);

							if (creepz->GetHealth() <= 0)
							{
								creepz->SetActive(false);
								player->SetGold(player->GetGold() + ((rand() % 10 + 1) * creepz->GetLevel()));
								tEnemyProgress->SetEnemyCounter(tEnemyProgress->GetEnemyCounter() - 1);
								enemycounter--;
								break;
							}
						}
					}
				}
			}
		}
	}

	// Despawn creep if bullet collides
	for (std::vector<Bullet *>::iterator it3 = bulletList.begin(); it3 != bulletList.end(); ++it3)
	{
		Bullet *bullet = *it3;
		if (bullet->GetActive())
		{
			for (std::vector<Enemy *>::iterator it2 = enemyList.begin(); it2 != enemyList.end(); ++it2)
			{
				Enemy *creep = *it2;
				if (creep->GetActive() && bullet->type != Bullet::GO_CANNONBULLET && creep->GetPos().x - bullet->GetPos().x < bullet->GetRadius().x && abs(creep->GetPos().y - bullet->GetPos().y) < bullet->GetRadius().y)
				{
					if (bullet->GetHealth() > 0)
					{
						bullet->SetHealth(bullet->GetHealth() - 1);
						bullet->SetPos(bullet->GetPos() + 20);
					}
					else if (bullet->GetHealth() <= 0)
					{
						bullet->SetActive(false);
					}
					if (bullet->type == Bullet::GO_SLOWBULLET)
					{
						creep->SetBuff(2);
						if (creep->GetBuff() > 0)
						{
							creep->SetVel(Vector3(-7.5f, 0, 0));
							creep->SetBuff(creep->GetBuff() - dt);
						}
						else if (creep->GetBuff() <= 0)
						{
							creep->SetVel(Vector3(-15, 0, 0));
						}
					}

					creep->SetHealth(creep->GetHealth() - bullet->GetDamage());

					if (creep->GetHealth() <= 0)
					{
						soundTypes(creep->type, true);
						creep->SetActive(false);
						player->SetGold(player->GetGold() + 50);
						tEnemyProgress->SetEnemyCounter(tEnemyProgress->GetEnemyCounter() - 1);
						enemycounter--;
						break;
					}
				}
			}
		}
	}

	// Spawn bomb for cannon (radius)
	for (std::vector<Bullet *>::iterator it3 = bulletList.begin(); it3 != bulletList.end(); ++it3)
	{
		Bullet *bullet = *it3;
		if (bullet->GetActive() && bullet->type == Bullet::GO_CANNONBULLET)
		{
			for (std::vector<Enemy *>::iterator it2 = enemyList.begin(); it2 != enemyList.end(); ++it2)
			{
				Enemy *creep = *it2;
				if (creep->GetActive())
				{
					Vector3 temp;
					temp.x = creep->GetPos().x;
					temp.y = creep->GetPos().y;

					if ((temp - bullet->GetPos()).Length() < 10)
					{
						creep->SetHealth(creep->GetHealth() - bullet->GetDamage());
						bullet->type = Bullet::GO_BOMBBULLET;
						if (creep->GetHealth() <= 0)
						{
							creep->SetActive(false);
							soundTypes(creep->type, true);
							enemycounter--;
							tEnemyProgress->SetEnemyCounter(tEnemyProgress->GetEnemyCounter() - 1);
							player->SetGold(player->GetGold() + 50);
							break;
						}
					}
				}
			}
		}
	}

	// Check if creep in range
	for (std::vector<Enemy *>::iterator it2 = enemyList.begin(); it2 != enemyList.end(); ++it2)
	{
		Enemy *creep = *it2;
		if (creep->GetActive() == true)
		{
			creep->SetFire(false);
			for (std::vector<Tower *>::iterator it = towerList.begin(); it != towerList.end(); ++it)
			{
				Tower *tower = *it;
				if (tower->GetActive() == true && tower->GetPos().y == creep->GetPos().y && tower->GetPos().x - creep->GetPos().x > -creep->GetRange() && creep->GetPos().x > tower->GetPos().x)
				{
					creep->SetFire(true);
					if (creep->GetActive() == true && creep->GetFire() == true)
					{
						if (creep->GetFireCounter() > 0)
						{
							creep->SetFireCounter(creep->GetFireCounter() - dt);
							break;
						}
						else if (creep->GetFireCounter() <= 0)
						{
							creep->SetFire(true);
							tower->SetHealth(tower->GetHealth() - creep->GetDamage());
							creep->SetFireCounter(creep->GetFireRate());
							if (tower->GetHealth() <= 0)
							{
								se->play2D("bin/sounds/towerDeath.mp3", false);
								se->setSoundVolume(0.25);
								int x = (int)((tower->GetPos().x / TILE_SIZE) - 0.5f);
								int y = (int)((tower->GetPos().y / TILE_SIZE) - 0.5f);
								theMap->theScreenMap[y][x] = 1;
								tower->SetActive(false);
								creep->SetFire(false);
							}
							break;
						}
					}
				}
			}
		}
	}
}

Bullet* CPlayState::FetchBullet()
{
	for (std::vector<Bullet *>::iterator it = bulletList.begin(); it != bulletList.end(); ++it)
	{
		Bullet *bullet = *it;
		if (!bullet->GetActive())
		{
			bullet->SetActive(true);
			return bullet;
		}
	}
	Bullet *bullet = new Bullet(Bullet::GO_NORMALBULLET);
	bullet->SetActive(true);
	bulletList.push_back(bullet);
	return bullet;
}

Enemy* CPlayState::FetchEnemy()
{
	for (std::vector<Enemy *>::iterator it = enemyList.begin(); it != enemyList.end(); ++it)
	{
		Enemy *creep = *it;
		if (!creep->GetActive())
		{
			creep->SetActive(true);
			return creep;
		}
	}
	Enemy *creep = new Enemy(Enemy::ENEMY_1);
	creep->SetActive(true);
	enemyList.push_back(creep);
	return creep;
}

Tower *CPlayState::FetchTower()
{
	for (std::vector<Tower *>::iterator it = towerList.begin(); it != towerList.end(); ++it)
	{
		Tower *tower = *it;
		if (!tower->GetActive())
		{
			tower->SetActive(true);
			return tower;
		}
	}
	Tower *tower = new Tower(Tower::TOWER_NORMAL);
	tower->SetActive(true);
	towerList.push_back(tower);
	return tower;
}

Spawn* CPlayState::FetchSpawn()
{
	for (std::vector<Spawn *>::iterator it = spawnList.begin(); it != spawnList.end(); ++it)
	{
		Spawn *spawn = *it;
		return spawn;
	}
	Spawn *spawn = new Spawn();
	spawnList.push_back(spawn);
	return spawn;
}

Powerup* CPlayState::FetchPower()
{
	for (std::vector<Powerup *>::iterator it = powerList.begin(); it != powerList.end(); ++it)
	{
		Powerup *power = *it;
		if (!power->GetActive())
		{
			power->SetActive(true);;
			return power;
		}
	}
	Powerup *power = new Powerup(Powerup::POWER_DOWN);
	power->SetActive(true);
	powerList.push_back(power);
	return power;

}

void CPlayState::DrawBullet(Bullet *bullet)
{
	switch (bullet->type)
	{
	case Bullet::GO_NORMALBULLET:
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glPushMatrix();
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindTexture(GL_TEXTURE_2D, BulletTexture[0].texID);
		glTranslatef(bullet->GetPos().x, bullet->GetPos().y, bullet->GetPos().z);
		//glScalef(go->scale.x, go->scale.y, go->scale.z);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0); glVertex2f(-12, -12);
		glTexCoord2f(0, 1); glVertex2f(-12, 12);
		glTexCoord2f(1, 1); glVertex2f(12, 12);
		glTexCoord2f(1, 0); glVertex2f(12, -12);
		glEnd();
		glPopMatrix();
		glDisable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);
		break;
	case Bullet::GO_CANNONBULLET:
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glPushMatrix();
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindTexture(GL_TEXTURE_2D, BulletTexture[1].texID);
		glTranslatef(bullet->GetPos().x, bullet->GetPos().y, bullet->GetPos().z);
		//glScalef(go->scale.x, go->scale.y, go->scale.z);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0); glVertex2f(-12, -12);
		glTexCoord2f(0, 1); glVertex2f(-12, 12);
		glTexCoord2f(1, 1); glVertex2f(12, 12);
		glTexCoord2f(1, 0); glVertex2f(12, -12);
		glEnd();
		glPopMatrix();
		glDisable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);
		break;
	case Bullet::GO_SLOWBULLET:
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glPushMatrix();
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindTexture(GL_TEXTURE_2D, BulletTexture[2].texID);
		glTranslatef(bullet->GetPos().x, bullet->GetPos().y, bullet->GetPos().z);
		//glScalef(go->scale.x, go->scale.y, go->scale.z);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0); glVertex2f(-12, -12);
		glTexCoord2f(0, 1); glVertex2f(-12, 12);
		glTexCoord2f(1, 1); glVertex2f(12, 12);
		glTexCoord2f(1, 0); glVertex2f(12, -12);
		glEnd();
		glPopMatrix();
		glDisable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);
		break;
	case Bullet::GO_SHOCKBULLET:
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glPushMatrix();
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindTexture(GL_TEXTURE_2D, BulletTexture[3].texID);
		glTranslatef(bullet->GetPos().x, bullet->GetPos().y, bullet->GetPos().z);
		//glScalef(go->scale.x, go->scale.y, go->scale.z);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0); glVertex2f(-12, -12);
		glTexCoord2f(0, 1); glVertex2f(-12, 12);
		glTexCoord2f(1, 1); glVertex2f(12, 12);
		glTexCoord2f(1, 0); glVertex2f(12, -12);
		glEnd();
		glPopMatrix();
		glDisable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);
		break;
	case Bullet::GO_BOMBBULLET:
		glPushMatrix();
		glColor3f(1.0f, 0.0f, 1.0f);
		glTranslatef(bullet->GetPos().x, bullet->GetPos().y, bullet->GetPos().z);
		//glScalef(go->scale.x, go->scale.y, go->scale.z);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0); glVertex2f(-75, -75);
		glTexCoord2f(0, 1); glVertex2f(-75, 75);
		glTexCoord2f(1, 1); glVertex2f(75, 75);
		glTexCoord2f(1, 0); glVertex2f(75, -75);
		glEnd();
		glPopMatrix();
		break;
	}
}

void CPlayState::DrawEnemy(Enemy *creep)
{
	int time = glutGet(GLUT_ELAPSED_TIME);
	static int ctime = glutGet(GLUT_ELAPSED_TIME);
	switch (creep->type)
	{
	case Enemy::ENEMY_1:
		creep->DrawHealthBar();
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glPushMatrix();
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindTexture(GL_TEXTURE_2D, CreepTexture[0].texID);
		glTranslatef(creep->GetPos().x, creep->GetPos().y, 0);
		if (time - ctime > 200) // the more it is the slower it becomes
		{
			heroAnimationCounter--;
			if (heroAnimationCounter == 0)
			{
				heroAnimationCounter = 2;
			}
			ctime = time;
		}

		glBegin(GL_QUADS);
		glTexCoord2f(0.5 * heroAnimationCounter + 0.5, 1); glVertex2f(-TILE_SIZE / 2, -TILE_SIZE / 2);
		glTexCoord2f(0.5 * heroAnimationCounter + 0.5, 0); glVertex2f(-TILE_SIZE / 2, TILE_SIZE / 2);
		glTexCoord2f(0.5 * heroAnimationCounter, 0); glVertex2f(TILE_SIZE / 2, TILE_SIZE / 2);
		glTexCoord2f(0.5 * heroAnimationCounter, 1); glVertex2f(TILE_SIZE / 2, -TILE_SIZE / 2);
		glEnd();
		glPopMatrix();
		glDisable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);
		break;
	case Enemy::ENEMY_2:
		creep->DrawHealthBar();
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glPushMatrix();
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindTexture(GL_TEXTURE_2D, CreepTexture[1].texID);
		glTranslatef(creep->GetPos().x, creep->GetPos().y, 0);
		if (time - ctime > 200) // the more it is the slower it becomes
		{
			heroAnimationCounter--;
			if (heroAnimationCounter == 0)
			{
				heroAnimationCounter = 2;
			}
			ctime = time;
		}

		glBegin(GL_QUADS);
		glTexCoord2f(0.16667 * heroAnimationCounter + 0.16667, 1); glVertex2f(-TILE_SIZE / 2, -TILE_SIZE / 2);
		glTexCoord2f(0.16667 * heroAnimationCounter + 0.16667, 0); glVertex2f(-TILE_SIZE / 2, TILE_SIZE / 2);
		glTexCoord2f(0.16667 * heroAnimationCounter, 0); glVertex2f(TILE_SIZE / 2, TILE_SIZE / 2);
		glTexCoord2f(0.16667 * heroAnimationCounter, 1); glVertex2f(TILE_SIZE / 2, -TILE_SIZE / 2);
		glEnd();
		glPopMatrix();
		glDisable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);
		break;
	case Enemy::ENEMY_3:
		creep->DrawHealthBar();
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glPushMatrix();
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindTexture(GL_TEXTURE_2D, CreepTexture[2].texID);
		glTranslatef(creep->GetPos().x, creep->GetPos().y, 0);
		if (time - ctime > 200) // the more it is the slower it becomes
		{
			heroAnimationCounter--;
			if (heroAnimationCounter == 0)
			{
				heroAnimationCounter = 6;
			}

			ctime = time;
		}

		glBegin(GL_QUADS);
		glTexCoord2f(0.16667 * heroAnimationCounter, 1);
		glVertex2f(-TILE_SIZE / 2, -TILE_SIZE / 2);
		glTexCoord2f(0.16667 * heroAnimationCounter, 0);
		glVertex2f(-TILE_SIZE / 2, TILE_SIZE / 2);
		glTexCoord2f(0.16667 * heroAnimationCounter + 0.16667, 0);
		glVertex2f(TILE_SIZE / 2, TILE_SIZE / 2);
		glTexCoord2f(0.16667 * heroAnimationCounter + 0.16667, 1);
		glVertex2f(TILE_SIZE / 2, -TILE_SIZE / 2);
		glEnd();
		glPopMatrix();
		glDisable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);
		break;
	}
}

void CPlayState::DrawTower(Tower *tower)
{
	tower->DrawHealthBar();
	tower->DrawLevel();
	switch (tower->type)
	{
	case Tower::TOWER_NORMAL:
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glPushMatrix();
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindTexture(GL_TEXTURE_2D, TowerTexture[0].texID);
		glTranslatef(tower->GetPos().x, tower->GetPos().y, 0);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0); glVertex2f(TILE_SIZE / 2, TILE_SIZE / 2);
		glTexCoord2f(0, 1); glVertex2f(TILE_SIZE / 2, -TILE_SIZE / 2);
		glTexCoord2f(1, 1); glVertex2f(-TILE_SIZE / 2, -TILE_SIZE / 2);
		glTexCoord2f(1, 0); glVertex2f(-TILE_SIZE / 2, TILE_SIZE / 2);
		glEnd();
		glPopMatrix();
		glDisable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);
		break;
	case Tower::TOWER_CANNON:
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glPushMatrix();
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindTexture(GL_TEXTURE_2D, TowerTexture[1].texID);
		glTranslatef(tower->GetPos().x, tower->GetPos().y, 0);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0); glVertex2f(TILE_SIZE / 2, TILE_SIZE / 2);
		glTexCoord2f(0, 1); glVertex2f(TILE_SIZE / 2, -TILE_SIZE / 2);
		glTexCoord2f(1, 1); glVertex2f(-TILE_SIZE / 2, -TILE_SIZE / 2);
		glTexCoord2f(1, 0); glVertex2f(-TILE_SIZE / 2, TILE_SIZE / 2);
		glEnd();
		glPopMatrix();
		glDisable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);
		break;
	case Tower::TOWER_SLOW:
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glPushMatrix();
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindTexture(GL_TEXTURE_2D, TowerTexture[2].texID);
		glTranslatef(tower->GetPos().x, tower->GetPos().y, 0);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0); glVertex2f(TILE_SIZE / 2, TILE_SIZE / 2);
		glTexCoord2f(0, 1); glVertex2f(TILE_SIZE / 2, -TILE_SIZE / 2);
		glTexCoord2f(1, 1); glVertex2f(-TILE_SIZE / 2, -TILE_SIZE / 2);
		glTexCoord2f(1, 0); glVertex2f(-TILE_SIZE / 2, TILE_SIZE / 2);
		glEnd();
		glPopMatrix();
		glDisable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);
		break;
	case Tower::TOWER_SHOCK:
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glPushMatrix();
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindTexture(GL_TEXTURE_2D, TowerTexture[3].texID);
		glTranslatef(tower->GetPos().x, tower->GetPos().y, 0);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0); glVertex2f(TILE_SIZE / 2, TILE_SIZE / 2);
		glTexCoord2f(0, 1); glVertex2f(TILE_SIZE / 2, -TILE_SIZE / 2);
		glTexCoord2f(1, 1); glVertex2f(-TILE_SIZE / 2, -TILE_SIZE / 2);
		glTexCoord2f(1, 0); glVertex2f(-TILE_SIZE / 2, TILE_SIZE / 2);
		glEnd();
		glPopMatrix();
		glDisable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);
		break;
	}
}

void CPlayState::RenderStringOnScreen(float x, float y, const char* quote)
{
	int length = strlen(quote);
	glRasterPos2f(x, y);

	for (int i = 0; i < length; i++)
	{
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, quote[i]);
	}
}

void CPlayState::RenderBackground(void)
{
	glEnable(GL_TEXTURE_2D);
	// Draw Background image

	glPushMatrix();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D, BackgroundTexture[0].texID);
	glPushMatrix();
	glBegin(GL_QUADS);
	//int height = SCREEN_HEIGHT * 1.333 / 1.5;
	glTexCoord2f(0, 0); glVertex2f(0, SCREEN_HEIGHT);
	glTexCoord2f(1, 0); glVertex2f(SCREEN_WIDTH, SCREEN_HEIGHT);
	glTexCoord2f(1, 1); glVertex2f(SCREEN_WIDTH, 0);
	glTexCoord2f(0, 1); glVertex2f(0, 0);
	glEnd();
	glPopMatrix();
	glDisable(GL_BLEND);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
}

void CPlayState::Load()
{
	ifstream inData;
	int length;

	string type;
	string value;

	inData.open("save/player.txt");
	while (!inData.eof()) {
		getline(inData, type, ',');
		getline(inData, value, '\n');
		{
			if (type == "life")
			{
				player->SetLife(stoi(value));
			}
			else if (type == "gold")
			{
				player->SetGold(stoi(value));
			}
			else if (type == "time")
			{
				spawntimer = stof(value);
			}
			else if (type == "progress")
			{
				progress = stoi(value);
			}
			else if (type == "enemyleft")
			{
				tEnemyProgress->SetEnemyCounter(stoi(value));
			}
			else if (type == "armageddon")
			{
				ClearMapCounter = stoi(value);
			}
			else if (type == "lasercannon")
			{
				ClearLaneCounter = stoi(value);
			}
		}
	}
	inData.close();

	inData.open("save/progress.txt");
	while (!inData.eof()) {
		getline(inData, type, ',');
		{
			if (type == "tower")
			{
				Tower *tower = new Tower();
				getline(inData, value, ',');
				tower->type = static_cast<Tower::TOWER_TYPE>(stoi(value));
				getline(inData, value, ',');
				tower->SetActive(stoi(value));
				getline(inData, value, ',');
				tower->SetRange(stoi(value));
				getline(inData, value, ',');
				tower->SetDamage(stoi(value));
				getline(inData, value, ',');
				tower->SetVel(Vector3(stoi(value), 0, 0));
				getline(inData, value, ',');
				tower->SetVel(Vector3(tower->GetVel().x, stoi(value), 0));
				getline(inData, value, ',');
				tower->SetVel(Vector3(tower->GetVel().x, tower->GetVel().y, stoi(value)));
				getline(inData, value, ',');
				tower->SetPos(Vector3(stoi(value), 0, 0));
				getline(inData, value, ',');
				tower->SetPos(Vector3(tower->GetPos().x, stoi(value), 0));
				getline(inData, value, ',');
				tower->SetPos(Vector3(tower->GetPos().x, tower->GetPos().y, stoi(value)));
				getline(inData, value, ',');
				tower->SetHealth(stoi(value));
				getline(inData, value, ',');
				tower->SetFireRate(stof(value));
				getline(inData, value, ',');
				tower->SetFireCounter(stof(value));
				getline(inData, value, '\n');
				tower->SetFire(stoi(value));
				towerList.push_back(tower);
				int x = (int)((tower->GetPos().x * 10 / WX) - 0.5f);
				int y = (int)((tower->GetPos().y * 7 / WY) - 0.5f);
				theMap->theScreenMap[y][x] = 3;

			}
			else if (type == "enemy")
			{
				Enemy *enemy = new Enemy();
				getline(inData, value, ',');
				enemy->type = static_cast<Enemy::ENEMY_TYPE>(stoi(value));
				getline(inData, value, ',');
				enemy->SetActive(stoi(value));
				getline(inData, value, ',');
				enemy->SetRange(stoi(value));
				getline(inData, value, ',');
				enemy->SetDamage(stoi(value));
				getline(inData, value, ',');
				enemy->SetVel(Vector3(stoi(value), 0, 0));
				getline(inData, value, ',');
				enemy->SetVel(Vector3(enemy->GetVel().x, stoi(value), 0));
				getline(inData, value, ',');
				enemy->SetVel(Vector3(enemy->GetVel().x, enemy->GetVel().y, stoi(value)));
				getline(inData, value, ',');
				enemy->SetPos(Vector3(stoi(value), 0, 0));
				getline(inData, value, ',');
				enemy->SetPos(Vector3(enemy->GetPos().x, stoi(value), 0));
				getline(inData, value, ',');
				enemy->SetPos(Vector3(enemy->GetPos().x, enemy->GetPos().y, stoi(value)));
				getline(inData, value, ',');
				enemy->SetHealth(stoi(value));
				getline(inData, value, ',');
				enemy->SetFireCounter(stof(value));
				getline(inData, value, '\n');
				enemy->SetFire(stoi(value));
				enemyList.push_back(enemy);
			}
			else if (type == "bullet")
			{
				Bullet *bullet = new Bullet();
				getline(inData, value, ',');
				bullet->type = static_cast<Bullet::BULLET_TYPE>(stoi(value));
				getline(inData, value, ',');
				bullet->SetActive(stoi(value));
				getline(inData, value, ',');
				bullet->SetVel(Vector3(stoi(value), 0, 0));
				getline(inData, value, ',');
				bullet->SetVel(Vector3(bullet->GetVel().x, stoi(value), 0));
				getline(inData, value, ',');
				bullet->SetVel(Vector3(bullet->GetVel().x, bullet->GetVel().y, stoi(value)));
				getline(inData, value, ',');
				bullet->SetPos(Vector3(stoi(value), 0, 0));
				getline(inData, value, ',');
				bullet->SetPos(Vector3(bullet->GetPos().x, stoi(value), 0));
				getline(inData, value, ',');
				bullet->SetPos(Vector3(bullet->GetPos().x, bullet->GetPos().y, stoi(value)));
				getline(inData, value, ',');
				bullet->SetRadius(Vector3(stoi(value), 0, 0));
				getline(inData, value, ',');
				bullet->SetRadius(Vector3(bullet->GetRadius().x, stoi(value), 0));
				getline(inData, value, ',');
				bullet->SetRadius(Vector3(bullet->GetRadius().x, bullet->GetRadius().y, stoi(value)));
				getline(inData, value, ',');
				bullet->SetHealth(stoi(value));
				getline(inData, value, '\n');
				bullet->SetDamage(stoi(value));
				bulletList.push_back(bullet);
			}
			else if (type == "spawnlist")
			{
				Spawn *spawn = new Spawn();
				getline(inData, value, ',');
				spawn->SetType(stoi(value));
				getline(inData, value, '\n');
				spawn->SetTime(stof(value));
				spawnList.push_back(spawn);
			}
		}
	}
	inData.seekg(0, ios::binary);
	length = inData.tellg();
	if (length == 0)
	{
		progress = 1;
		LoadSpawn();
	}
	inData.close();
}

void CPlayState::Save()
{
	ofstream file("save/player.txt");
	if (file.is_open())
	{
		file << "life, " << player->GetLife() << "\n";
		file << "gold, " << player->GetGold() << "\n";
		file << "time, " << spawntimer << "\n";
		file << "progress, " << progress << "\n";
		file << "enemyleft, " << tEnemyProgress->GetEnemyCounter() << "\n";
		file << "armageddon, " << ClearMapCounter << "\n";
		file << "lasercannon, " << ClearLaneCounter << "\n";
		file.close();
	}
	else
	{
		cout << "Unable to save stats!" << endl;
	}

	ofstream file2("save/progress.txt");
	if (file2.is_open())
	{
		Tower *tower = FetchTower();
		for (unsigned int i = 0; i < towerList.size(); ++i)
		{
			if (towerList[i]->GetActive())
			{
				if (towerList[i]->GetPos().x != 0)
				{
					file2 << "tower, " << towerList[i]->type << ", " << towerList[i]->GetActive() << ", " << towerList[i]->GetRange() << ", " << towerList[i]->GetDamage() << ", "
						<< towerList[i]->GetVel().x << ", " << towerList[i]->GetVel().y << ", " << towerList[i]->GetVel().z << ", "
						<< towerList[i]->GetPos().x << ", " << towerList[i]->GetPos().y << ", " << towerList[i]->GetPos().z << ", "
						<< towerList[i]->GetHealth() << ", " << towerList[i]->GetFireRate() << ", " << towerList[i]->GetFireCounter() << ", " << towerList[i]->GetFire() << "\n";
				}
			}
		}
		Enemy *creep = FetchEnemy();
		for (unsigned int j = 0; j < enemyList.size(); ++j)
		{
			if (enemyList[j]->GetActive())
			{
				if (enemyList[j]->GetPos().x != 0)
				{
					file2 << "enemy, " << enemyList[j]->type << ", " << enemyList[j]->GetActive() << ", " << enemyList[j]->GetRange() << ", " << enemyList[j]->GetDamage() << ", "
						<< enemyList[j]->GetVel().x << ", " << enemyList[j]->GetVel().y << ", " << enemyList[j]->GetVel().z << ", "
						<< enemyList[j]->GetPos().x << ", " << enemyList[j]->GetPos().y << ", " << enemyList[j]->GetPos().z << ", "
						<< enemyList[j]->GetHealth() << ", " << enemyList[j]->GetFireRate() << ", " << enemyList[j]->GetFireCounter() << ", " << enemyList[j]->GetFire() << "\n";
				}
			}
		}
		Bullet *bullet = FetchBullet();
		for (unsigned int k = 0; k < bulletList.size(); ++k)
		{
			if (bulletList[k]->GetActive())
			{
				if (bulletList[k]->GetPos().x != 0)
				{
					file2 << "bullet, " << bulletList[k]->type << ", " << bulletList[k]->GetActive() << ", "
						<< bulletList[k]->GetVel().x << ", " << bulletList[k]->GetVel().y << ", " << bulletList[k]->GetVel().z << ", "
						<< bulletList[k]->GetPos().x << ", " << bulletList[k]->GetPos().y << ", " << bulletList[k]->GetPos().z << ", "
						<< bulletList[k]->GetRadius().x << ", " << bulletList[k]->GetRadius().y << ", " << bulletList[k]->GetRadius().z << ", "
						<< bulletList[k]->GetHealth() << ", " << bulletList[k]->GetDamage() << "\n";
				}
			}
		}

		Spawn *spawn = FetchSpawn();
		for (unsigned int x = 0; x < spawnList.size(); ++x)
		{
			if (spawnList[x]->GetTime() >= spawntimer)
			{
				file2 << "spawnlist, " << spawnList[x]->GetType() << ", " << spawnList[x]->GetTime() << "\n";
			}
		}

		file2.close();
	}
	else
	{
		cout << "Unable to save progress!" << endl;
	}
}

void CPlayState::UpdateSpawn()
{
	for (std::vector<Spawn *>::iterator it = spawnList.begin(); it != spawnList.end();)
	{
		Spawn *spawn = *it;
		if (spawn->GetTime() <= spawntimer)
		{
			if (Enemy *creep = FetchEnemy())
			{
				creep->type = static_cast<Enemy::ENEMY_TYPE>(spawn->GetType());
				creep->SetAtt(enemyClone[creep->type - 1]->GetFireRate(), enemyClone[creep->type - 1]->GetDamage(),
					enemyClone[creep->type - 1]->GetRange(), enemyClone[creep->type - 1]->GetHealth(), enemyClone[creep->type - 1]->GetSpeed());
				creep->SetVel(Vector3(-15 * creep->GetSpeed(), 0, 0));
				creep->SetPos(Vector3(SCREEN_WIDTH, ((rand() % 5 + 1) + 0.5f) * 96, 0));
			}
			it = spawnList.erase(it);
		}
		else
			++it;
	}
}

void CPlayState::LoadSpawn()
{
	ifstream inData;

	string index;
	string time;

	if (progress == 1)
	{
		inData.open("save/spawn.txt");
	}
	else if (progress == 2)
	{
		inData.open("save/spawn2.txt");
	}
	else if (progress == 3)
	{
		inData.open("save/spawn3.txt");
	}
	else if (progress == 4)
	{
		inData.open("save/spawn4.txt");
	}
	else if (progress == 5)
	{
		inData.open("save/spawn5.txt");
	}
	else if (progress == 6)
	{
		inData.open("save/spawn6.txt");
	}

	while (!inData.eof()) {
		Spawn *spawn = new Spawn();

		getline(inData, index, ',');
		getline(inData, time, '\n');

		spawn->SetType(stoi(index));
		spawn->SetTime(stof(time));
		tEnemyProgress->SetEnemyCounter(tEnemyProgress->GetEnemyCounter() + 1);
		enemycounter++;
		spawnList.push_back(spawn);
	}
	inData.close();
}

void CPlayState::LoadAtt()
{
	ifstream inData;

	string index;
	string value;

	inData.open("save/tower.txt");
	while (!inData.eof())
	{
		Tower *tower = new Tower();

		getline(inData, index, ',');
		tower->type = static_cast<Tower::TOWER_TYPE>(stoi(index));
		getline(inData, value, ',');
		tower->SetFireRate(stof(value));
		getline(inData, value, ',');
		tower->SetCost(stoi(value));
		getline(inData, value, ',');
		tower->SetDamage(stoi(value));
		getline(inData, value, ',');
		tower->SetRange(stoi(value));
		getline(inData, value, '\n');
		tower->SetHealth(stoi(value));
		towerClone.push_back(tower);
	}
	inData.close();

	inData.open("save/creep.txt");
	while (!inData.eof())
	{
		Enemy *creep = new Enemy();

		getline(inData, index, ',');
		creep->type = static_cast<Enemy::ENEMY_TYPE>(stoi(index));
		getline(inData, value, ',');
		creep->SetFireRate(stof(value));
		getline(inData, value, ',');
		creep->SetDamage(stoi(value));
		getline(inData, value, ',');
		creep->SetRange(stoi(value));
		getline(inData, value, ',');
		creep->SetHealth(stoi(value));
		getline(inData, value, '\n');
		creep->SetSpeed(stof(value));
		enemyClone.push_back(creep);
	}
	inData.close();
}

void CPlayState::playSound(void)
{
	if (soundon == true)
	{
		sound.setFileName("bin/sounds/gameTheme_3.mp3");
		//sound.setVolume(50);
		sound.playSoundThreaded();
	}

}

void CPlayState::shooting(bool firing)
{
	se->play2D("bin/sounds/smb2_pickup.wav", false);
	se->setSoundVolume(0.3);
	firing = false;
}

void CPlayState::soundTypes(int type, bool death)
{
	if (type == 1)
	{
		se->play2D("bin/sounds/chipDeath.mp3", false);
		se->setSoundVolume(0.25);
		death = false;
	}
	else if (type == 2)
	{
		se->play2D("bin/sounds/cardDeath.mp3", false);
		se->setSoundVolume(0.25);
		death = false;
	}
	else if (type == 3)
	{
		se->play2D("bin/sounds/humanDeath.mp3", false);
		se->setSoundVolume(0.25);
		death = false;
	}
	else if (type == 4)
	{
		se->play2D("bin/sounds/xplosionSFX.wav", false);
		se->setSoundVolume(0.25);
		death = false;
	}
	else if (type == 5)
	{
		se->play2D("bin/sounds/laserSFX.mp3", false);
		se->setSoundVolume(0.25);
		death = false;
	}
}

void CPlayState::loadlevel()
{
	if (progress == 1)
	{
		theMap->LoadMap("bin/maps/MapDesign.csv");
	}

	else if (progress == 2)
	{
		theMap->LoadMap("bin/maps/MapDesign2.csv");
	}

	else if (progress == 3)
	{
		theMap->LoadMap("bin/maps/MapDesign3.csv");
	}

	else if (progress == 4)
	{
		theMap->LoadMap("bin/maps/MapDesign4.csv");
	}

	else if (progress == 5)
	{
		theMap->LoadMap("bin/maps/MapDesign5.csv");
	}

	else if (progress == 6)
	{
		theMap->LoadMap("bin/maps/MapDesign6.csv");
	}
}

void CPlayState::clearmap()
{
	for (std::vector <Tower*>::iterator it = towerList.begin(); it < towerList.end(); it++)
	{
		Tower *tower = *it;
		if (tower->GetActive())
		{
			tower->SetActive(false);
		}
	}
	for (std::vector <Enemy*>::iterator it = enemyList.begin(); it < enemyList.end(); it++)
	{
		Enemy *creep = *it;
		if (creep->GetActive())
		{
			creep->SetActive(false);
		}
	}
	for (std::vector <Bullet*>::iterator it = bulletList.begin(); it < bulletList.end(); it++)
	{
		Bullet *bullet = *it;
		if (bullet->GetActive())
		{
			bullet->SetActive(false);
		}
	}
}

void CPlayState::track()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	//int w = glutGet(GLUT_WINDOW_WIDTH);
	//int h = glutGet(GLUT_WINDOW_HEIGHT);
	/*glScalef(w* (w/1313), h * (h/697),0);*/
	gluOrtho2D(0.0, w, 0.0, h);

	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_TEXTURE_2D);

	glColor3f(1.0f, 1.0f, 1.0f);

	glPushMatrix();
	glLoadIdentity();
	glBindTexture(GL_TEXTURE_2D, Music[0].texID);


	glTranslatef((w * 0.5f), (h * 0.5f), 0);
	glBegin(GL_QUADS);

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-w * 0.2f, h * 0.3f, 0);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-w * 0.2f, -h * 0.3f, 0);

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(w * 0.2f, -h * 0.3f, 0);

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(w * 0.2f, h * 0.3f, 0);
	glEnd();
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

void CPlayState::tower1()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	//int w = glutGet(GLUT_WINDOW_WIDTH);
	//int h = glutGet(GLUT_WINDOW_HEIGHT);
	gluOrtho2D(0.0, w, 0.0, h);

	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_TEXTURE_2D);

	glPushMatrix();
	glLoadIdentity();
	glBindTexture(GL_TEXTURE_2D, Icon[0].texID);

	glTranslatef((w * 0.35f), (h * 0.926f), 0);
	glBegin(GL_QUADS);

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-w / 20, h / 14, 0);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-w / 20, -h / 14, 0);

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(w / 20, -h / 14, 0);

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(w / 20, h / 14, 0);
	glEnd();
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

void CPlayState::tower2()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	//int w = glutGet(GLUT_WINDOW_WIDTH);
	//int h = glutGet(GLUT_WINDOW_HEIGHT);
	gluOrtho2D(0.0, w, 0.0, h);

	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_TEXTURE_2D);

	glPushMatrix();
	glLoadIdentity();
	glBindTexture(GL_TEXTURE_2D, Icon[1].texID);

	glTranslatef((w * 0.45f), (h * 0.926f), 0);
	glBegin(GL_QUADS);

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-w / 20, h / 14, 0);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-w / 20, -h / 14, 0);

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(w / 20, -h / 14, 0);

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(w / 20, h / 14, 0);
	glEnd();
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

void CPlayState::tower3()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	//int w = glutGet(GLUT_WINDOW_WIDTH);
	//int h = glutGet(GLUT_WINDOW_HEIGHT);
	gluOrtho2D(0.0, w, 0.0, h);

	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_TEXTURE_2D);

	glPushMatrix();
	glLoadIdentity();
	glBindTexture(GL_TEXTURE_2D, Icon[2].texID);

	glTranslatef((w * 0.55f), (h * 0.926f), 0);
	glBegin(GL_QUADS);

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-w / 20, h / 14, 0);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-w / 20, -h / 14, 0);

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(w / 20, -h / 14, 0);

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(w / 20, h / 14, 0);
	glEnd();
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

void CPlayState::tower4()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	//int w = glutGet(GLUT_WINDOW_WIDTH);
	//int h = glutGet(GLUT_WINDOW_HEIGHT);
	gluOrtho2D(0.0, w, 0.0, h);

	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_TEXTURE_2D);

	glPushMatrix();
	glLoadIdentity();
	glBindTexture(GL_TEXTURE_2D, Icon[3].texID);

	glTranslatef((w * 0.65f), (h * 0.926f), 0);
	glBegin(GL_QUADS);

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-w / 20, h / 14, 0);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-w / 20, -h / 14, 0);

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(w / 20, -h / 14, 0);

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(w / 20, h / 14, 0);
	glEnd();
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

void CPlayState::power1()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	int w = glutGet(GLUT_WINDOW_WIDTH);
	int h = glutGet(GLUT_WINDOW_HEIGHT);
	gluOrtho2D(0.0, w, 0.0, h);

	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glPushMatrix();
	glLoadIdentity();
	glBindTexture(GL_TEXTURE_2D, Power[0].texID);

	glTranslatef((w * 0.85f), (h * 0.926f), 0);
	glBegin(GL_QUADS);

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-w / 20, h / 14, 0);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-w / 20, -h / 14, 0);

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(w / 20, -h / 14, 0);

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(w / 20, h / 14, 0);
	glEnd();
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glPopMatrix();
}

void CPlayState::power2()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	int w = glutGet(GLUT_WINDOW_WIDTH);
	int h = glutGet(GLUT_WINDOW_HEIGHT);
	gluOrtho2D(0.0, w, 0.0, h);

	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glPushMatrix();
	glLoadIdentity();
	glBindTexture(GL_TEXTURE_2D, Power[1].texID);

	glTranslatef((w * 0.95f), (h * 0.926f), 0);
	glBegin(GL_QUADS);

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-w / 20, h / 14, 0);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-w / 20, -h / 14, 0);

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(w / 20, -h / 14, 0);

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(w / 20, h / 14, 0);
	glEnd();
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glPopMatrix();
}

void CPlayState::RenderUpgrade(int x, int y)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	//int w = glutGet(GLUT_WINDOW_WIDTH);
	//int h = glutGet(GLUT_WINDOW_HEIGHT);
	gluOrtho2D(0.0, w, 0.0, h);

	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glPushMatrix();
	glLoadIdentity();
	glBindTexture(GL_TEXTURE_2D, Upgrade[0].texID);

	glTranslatef(x, y, 0);
	glBegin(GL_QUADS);

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-20, 20, 0);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-20, -20, 0);

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(20, -20, 0);

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(20, 20, 0);
	glEnd();
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

void CPlayState::RenderInfo(int x, int y)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0.0, w, 0.0, h);
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPushMatrix();
	glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
	glTranslatef(x, y, 0);
	glBegin(GL_QUADS);
	glVertex2f(-10, 20);
	glVertex2f(-10, -150);
	glVertex2f(150, -150);
	glVertex2f(150, 20);
	glEnd();
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	glColor3f(1.0f, 1.0f, 1.0f);
	char temp[256];
	if (info > 0)
	{
		if (info == 1)
		{
			sprintf_s(temp, "Normal Tower");
		}
		else if (info == 2)
		{
			sprintf_s(temp, "Cannon Tower");
		}
		else if (info == 3)
		{
			sprintf_s(temp, "Lightning Tower");
		}
		else if (info == 4)
		{
			sprintf_s(temp, "Slow Tower");
		}
		RenderStringOnScreen((float)mouseInfo.lastX / w * SCREEN_WIDTH, (float)mouseInfo.lastY / h * SCREEN_HEIGHT, temp);
		sprintf_s(temp, "Health: %d (x 1.5)", towerClone[info - 1]->GetHealth());
		RenderStringOnScreen((float)mouseInfo.lastX / w * SCREEN_WIDTH, (float)mouseInfo.lastY / h * SCREEN_HEIGHT + 18, temp);
		sprintf_s(temp, "Cost: %d (+ 50)", towerClone[info - 1]->GetCost());
		RenderStringOnScreen((float)mouseInfo.lastX / w * SCREEN_WIDTH, (float)mouseInfo.lastY / h * SCREEN_HEIGHT + 36, temp);
		sprintf_s(temp, "Damage: %d (x 1.5)", towerClone[info - 1]->GetDamage());
		RenderStringOnScreen((float)mouseInfo.lastX / w * SCREEN_WIDTH, (float)mouseInfo.lastY / h * SCREEN_HEIGHT + 54, temp);
		sprintf_s(temp, "Firerate: %d", towerClone[info - 1]->GetFireRate());
		RenderStringOnScreen((float)mouseInfo.lastX / w * SCREEN_WIDTH, (float)mouseInfo.lastY / h * SCREEN_HEIGHT + 72, temp);
		sprintf_s(temp, "Range: %d", towerClone[info - 1]->GetRange());
		RenderStringOnScreen((float)mouseInfo.lastX / w * SCREEN_WIDTH, (float)mouseInfo.lastY / h * SCREEN_HEIGHT + 90, temp);
	}
}

void CPlayState::savegame()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	int w = glutGet(GLUT_WINDOW_WIDTH);
	int h = glutGet(GLUT_WINDOW_HEIGHT);
	/*glScalef(w* (w/1313), h * (h/697),0);*/
	gluOrtho2D(0.0, w, 0.0, h);

	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_TEXTURE_2D);

	glPushMatrix();
	glLoadIdentity();

	if (gamesave == 1)
	{
		glBindTexture(GL_TEXTURE_2D, Quit[0].texID);
	}

	if (gamesave == 2)
	{
		glBindTexture(GL_TEXTURE_2D, Quit[1].texID);
	}

	if (gamesave == 3)
	{
		glBindTexture(GL_TEXTURE_2D, Quit[2].texID);
	}

	glColor3f(1.0f, 1.0f, 1.0f);

	glTranslatef((w * 0.5f), (h * 0.5f), 0);
	glBegin(GL_QUADS);

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-w * 0.2f, h * 0.3f, 0);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-w * 0.2f, -h * 0.3f, 0);

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(w * 0.2f, -h * 0.3f, 0);

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(w * 0.2f, h * 0.3f, 0);
	glEnd();
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

void CPlayState::DrawHeart()
{
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D, Heart[0].texID);
	glPushMatrix();
	glTranslatef(0, 30, 0);
	glScalef(0.4, 0.4, 0.4);

	glBegin(GL_QUADS);
	int height = 100 * 1.333 / 1.5;
	glTexCoord2f(0, 0); glVertex2f(10, 10);
	glTexCoord2f(1, 0); glVertex2f(10, 10 + height);
	glTexCoord2f(1, 1); glVertex2f(110, 10 + height);
	glTexCoord2f(0, 1); glVertex2f(110, 10);
	glEnd();
	glPopMatrix();
	glDisable(GL_BLEND);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
}

void CPlayState::powerTex(bool yay, int boo)
{
	if (yay == true && boo == 1)
	{
		soundTypes(4, true);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		int w = glutGet(GLUT_WINDOW_WIDTH);
		int h = glutGet(GLUT_WINDOW_HEIGHT);
		/*glScalef(w* (w/1313), h * (h/697),0);*/
		gluOrtho2D(0.0, w, 0.0, h);

		glMatrixMode(GL_MODELVIEW);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(1.0, 1.0, 1.0, 1.0);
		glPushMatrix();
		glLoadIdentity();
		glBindTexture(GL_TEXTURE_2D, PowerBoom[0].texID);

		glTranslatef((w * 0.5f), (h * 0.5f), 0);
		//glTranslatef((w * 0.5f), (-h * 0.5f) + transition, 0);
		//glTranslatef((w * 0.95f), (h * 0.03f), 0);

		glBegin(GL_QUADS);

		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(-w * 0.5f, h * 0.5f, 0);

		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-w * 0.5f, -h * 0.5f, 0);

		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(w * 0.5f, -h * 0.5f, 0);

		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(w * 0.5f, h * 0.5f, 0);
		glEnd();
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
		glPopMatrix();

		yay = false;
	}
	else if (yay == true && boo == 2)
	{
		soundTypes(5, true);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		int w = glutGet(GLUT_WINDOW_WIDTH);
		int h = glutGet(GLUT_WINDOW_HEIGHT);
		/*glScalef(w* (w/1313), h * (h/697),0);*/
		gluOrtho2D(0.0, w, 0.0, h);

		glMatrixMode(GL_MODELVIEW);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(1.0, 1.0, 1.0, 1.0);
		glPushMatrix();
		glLoadIdentity();
		glBindTexture(GL_TEXTURE_2D, PowerBoom[1].texID);

		glTranslatef((w * 0.5f), (h - mouseInfo.lastY), 0);
		//glTranslatef((w * 0.5f), (-h * 0.5f) + transition, 0);
		//glTranslatef((w * 0.95f), (h * 0.03f), 0);

		glBegin(GL_QUADS);

		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(-w * 0.5f, h * 0.2f, 0);

		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-w * 0.5f, -h * 0.2f, 0);

		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(w * 0.5f, -h * 0.2f, 0);

		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(w * 0.5f, h * 0.2f, 0);
		glEnd();
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
		glPopMatrix();
		yay = false;
	}

}