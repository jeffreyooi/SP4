#include "Tank.h"

TextureImage Tank::body;

Tank::Tank(State state)
{
	this->state = STATE_IDLE;
	SetActive(false);
	SetDamage(10);
	SetRange(800);
	SetFire(false);
	SetFireRate(3.0f);
	SetPos(Vector3(-50, 336, 0));
	Target = NULL;
	inPosition = false;
	LoadTGA(&body, "bin/tower/Unit2_Tank.tga");
}

Tank::~Tank()
{

}

bool Tank::LoadTGA(TextureImage *texture, char *filename)			// Loads A TGA File Into Memory
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

void Tank::ChangeState()
{
	if (state == STATE_IDLE)
	{
		if (this->GetActive())
		{
			state = STATE_FALLIN;
		}
		if (inPosition)
		{
			if (Target != NULL && Target->GetActive() && Target->GetHealth() > 0)
			{
				Vector3 CurrentTarget = Target->GetPos() - this->GetPos();
				if ((this->GetRange() * this->GetRange()) >= CurrentTarget.Dot(CurrentTarget))
				{
					state = STATE_LOADING;
				}
			}
		}
	}
	else if (state == STATE_FALLIN)
	{
		if (this->GetPos().x >= 50)
		{
			inPosition = true;
			//this->SetPos(Vector3(50, this->GetPos().y, this->GetPos().z));
			if (Target == NULL)
			{
				state = STATE_IDLE;
			}
			else
			{
				state = STATE_LOADING;
			}
		}
	}
	else if (state == STATE_LOADING)
	{
		if (this->GetActive() && inPosition)
		{
			if (Target != NULL && Target->GetActive() && Target->GetHealth() > 0)
			{
				Vector3 CurrentTarget = Target->GetPos() - this->GetPos();
				if ((this->GetRange() * this->GetRange()) >= CurrentTarget.Dot(CurrentTarget))
				{
					if (this->GetFireCounter() <= 0.0f)
					{
						state = STATE_ATTACK;
						this->SetFireCounter(this->GetFireRate());
					}
				}
			}
			else
				state = STATE_IDLE;
		}
		else
			state = STATE_FALLOUT;
	}
	else if (state == STATE_ATTACK)
	{
		if (this->GetActive() && inPosition)
		{
			if (Target != NULL && Target->GetActive() && Target->GetHealth() > 0)
			{
				Vector3 CurrentTarget = Target->GetPos() - this->GetPos();
				//if ((this->GetRange() * this->GetRange()) >= CurrentTarget.Dot(CurrentTarget))
				//{
				state = STATE_LOADING;
				//}
			}
			else
				state = STATE_IDLE;
		}
		else
			state = STATE_FALLOUT;
	}
	else if (state == STATE_FALLOUT)
	{
		if (this->GetPos().x <= -50)
		{
			//this->SetPos(Vector3(0, this->GetPos().y, this->GetPos().z));
			state = STATE_IDLE;
			inPosition = false;
		}
	}
}

void Tank::Respond(float dt)
{
	switch (state)
	{
	case STATE_IDLE:
		break;
	case STATE_FALLIN:
		{
			if (!inPosition)
				this->SetPos(Vector3(this->GetPos().x + (50 * dt), this->GetPos().y, this->GetPos().z));
		}
		break;
	case STATE_LOADING:
		{
			this->SetFireCounter(this->GetFireCounter() - dt);
		}
		break;
	case STATE_ATTACK:
		{
			if (Target != NULL && Target->GetActive() && Target->GetHealth() > 0 && inPosition)
			{
				Bullet* bullet = new Bullet();
				bullet->SetActive(true);
				bullet->SetDamage(this->GetDamage());
				bullet->SetPos(this->GetPos());
				bullet->SetVel(Target->GetPos() - this->GetPos());
				bullet->SetSpeed(400);
				CPlayState::Instance()->GetBulletList().push_back(bullet);
			}
		}
		break;
	case STATE_FALLOUT:
		{
			if (inPosition)
				this->SetPos(Vector3(this->GetPos().x - (50 * dt), this->GetPos().y, this->GetPos().z));
		}
		break;
	}
}

void Tank::Update(float dt)
{
	ChangeState();
	Respond(dt);
}

void Tank::Render()
{
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D, body.texID);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glTranslatef(GetPos().x, GetPos().y, GetPos().z);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 1); glVertex2f(-96, -96);
	glTexCoord2f(1, 1); glVertex2f(96, -96);
	glTexCoord2f(1, 0); glVertex2f(96, 96);
	glTexCoord2f(0, 0); glVertex2f(-96, 96);
	glEnd();
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

void Tank::GetTarget(Enemy* enemy)
{
	if (Enemy *test = dynamic_cast<Enemy*>(enemy))
	{
		if (enemy == Target)
			return;
		else if (Target == NULL || !Target->GetActive())
		{
			Target = enemy;
			return;
		}
		else if (Target != NULL && Target->GetActive())
		{
			Vector3 CurrentTarget = Target->GetPos() - this->GetPos();
			Vector3 ThisTarget = enemy->GetPos() - this->GetPos();

			if (ThisTarget.Dot(ThisTarget) < CurrentTarget.Dot(CurrentTarget))
			{
				Target = enemy;
			}
		}
	}
}

void Tank::SetInPosition(bool inPosition)
{
	this->inPosition = inPosition;
}

bool Tank::GetInPosition()
{
	return inPosition;
}