/////////////////////////////////////////////////////
//	File Name:	ParticleEmitter.cpp
//	Date:		6/29/05
//	Author:		Will Dull
//	Purpose:	The base particle and emitter
//				classes implementations.
/////////////////////////////////////////////////////

#include "precompiled.h"
#include "ParticleEmitter.h"
#include "ParticleEngine.h"

#define PAR_LOG(a,b,c) LOG_SYS::GetInstance()->WriteC(a, LOG_SYS::OBJ, b, c)

CEmitter::CEmitter(const int MAX_PARTICLES, const int lifetime, int textureID)
{
	particleCount = 0;
	// declare the pool of nodes
	max_particles = MAX_PARTICLES;
	heap = new tParticle[max_particles];
	emitterLife = lifetime;
	decrementLife = true;
	decrementAlpha = true;
	renderParticles = true;
	isFinished = false;
	updateSpeed = 0.02f;
	blend_mode		= 1;
	size = 0.15f;
	texture = textureID;
	
	// init the used/open list
	usedList = NULL;
	openList = NULL;

	// link all the particles in the heap
	//	into one large open list
	for(int i = 0; i < max_particles - 1; i++)
	{
		heap[i].next = &(heap[i + 1]);	 
	}
	openList = heap;
}

CEmitter::~CEmitter(void)
{
	/*int open = 0;
	int used = 0;
	int final = 0;
	tParticle *iter = openList;
	while(iter)
	{
		open++;
		iter = iter->next;
	}

	iter = usedList;
	while(iter)
	{
		used++;
		iter = iter->next;
	}
	final = open + used;*/

	delete [] heap;
}

bool CEmitter::addParticle()
{
	tColor start, end;
	float fYaw, fPitch, fSpeed;

	if(!openList)
		return false;

	if(particleCount < max_particles)
	{
		// get a particle from the open list
		tParticle *particle = openList;

		// set it's initial position to the emitter's position
		particle->pos.x = pos.x;
		particle->pos.y = pos.y;
		particle->pos.z = pos.z;

		// Calculate the starting direction vector
		fYaw = yaw + (yawVar * RandomNum());
		fPitch = pitch + (pitchVar * RandomNum());

		// Convert the rotations to a vector
		RotationToDirection(fPitch,fYaw,&particle->dir);

		// Multiply in the speed factor
		fSpeed = speed + (speedVar * RandomNum());
		particle->dir.x *= fSpeed;
		particle->dir.y *= fSpeed;
		particle->dir.z *= fSpeed;

		// Calculate the life span
		particle->life = life + (int)((float)lifeVar * RandomNum());

		// Calculate the colors
		start.r = startColor.r + (startColorVar.r * RandomChar());
		start.g = startColor.g + (startColorVar.g * RandomChar());
		start.b = startColor.b + (startColorVar.b * RandomChar());
		end.r = endColor.r + (endColorVar.r * RandomChar());
		end.g = endColor.g + (endColorVar.g * RandomChar());
		end.b = endColor.b + (endColorVar.b * RandomChar());

		// set the initial color of the particle
		particle->color.r = start.r;
		particle->color.g = start.g;
		particle->color.b = start.b;

		// Create the color delta
		particle->deltaColor.r = (end.r - start.r) / particle->life;
		particle->deltaColor.g = (end.g - start.g) / particle->life;
		particle->deltaColor.b = (end.b - start.b) / particle->life;

		particle->alpha = 255.0f;
		particle->alphaDelta = particle->alpha / particle->life;

		particle->inPos = false;

		// Now, we pop a node from the open list and put it into the used list
		//tParticleNode *tempNode = openList; // get the top of the list that we have been filling in
		//openList = tempNode->next;
		//tempNode->next = usedList;			// have it link to the top of the used list
		//usedList = tempNode;				// set the new linked node as the start of the list
		openList = particle->next;
		particle->next = usedList;
		usedList = particle;

		// update the length of the used list (particle Count)
		particleCount++;
		return true;
	}

	return false;
}

bool CEmitter::renderEmitter()
{
	if(renderParticles)
	{
		switch(blend_mode)
		{
		case 1:
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);						// Fire
			break;
		case 2:
			glBlendFunc(GL_SRC_COLOR, GL_ONE);						// Crappy Fire
			break;
		case 3:
			glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);		// Plain Particles
			break;
		case 4:
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_COLOR);		// Nice fade out effect
			break;
		}

		// Bind the texture. Use the texture assigned to this emitter.
		glBindTexture(GL_TEXTURE_2D, texture);

		glBegin(GL_QUADS);
		{
			tParticle *tempParticle = usedList;
		
			while(tempParticle)
			{
				tColor *pColor = &(tempParticle->color);
				glColor4ub(pColor->r,pColor->g, pColor->b, (GLubyte)tempParticle->alpha);
				glTexCoord2d(0.0, 0.0);
				tVector *pPos = &(tempParticle->pos);
				glVertex3f(pPos->x - size, pPos->y + size, pPos->z);
				glTexCoord2d(0.0, 1.0);
				glVertex3f(pPos->x - size, pPos->y - size, pPos->z);
				glTexCoord2d(1.0, 1.0);
				glVertex3f(pPos->x + size, pPos->y - size, pPos->z);
				glTexCoord2d(1.0, 0.0);
				glVertex3f(pPos->x + size, pPos->y + size, pPos->z);
				tempParticle = tempParticle->next;
			}
		}
		glEnd();

		return true;
	}
	return false;
}
