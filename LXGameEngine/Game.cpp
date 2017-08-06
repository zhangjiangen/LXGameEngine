#include "stdafx.h"
#include "Game.h"
#include "Scene.h"
#include "Sprite.h"

Game * g_Game = nullptr;

Game::Game()
{
}

Game * Game::getInstance()
{
	if (g_Game == nullptr)
		g_Game = new (std::nothrow) Game;
	return g_Game;
}

bool Game::start()
{
	auto scene = Scene::getInstance();

	Sprite* sprite = new (std::nothrow) Sprite();
	sprite->initWithFile("Res/wall.jpg");
	//sprite->dump();
	sprite->setContentSize(512, 512);
	sprite->setPosition(512.0, 512.0f);
	sprite->setAnchorPoint(0.5f, 0.5f);
	sprite->setSwallowTouches(true);
	//sprite->setRotation(-45.0f);
	//sprite->setScale(1.0f, 2.0f);
	//sprite->visit(glm::mat4());
	scene->addChild(sprite);

	Sprite* sprite2 = new (std::nothrow) Sprite();
	sprite2->initWithFile("Res/test.png");
	sprite2->setPosition(0, 0);
	sprite2->setContentSize(512, 512);
	sprite2->autoRelease();
	sprite2->setLocalZ(-1);
	sprite->addChild(sprite2);

	Sprite* sprite3 = new (std::nothrow) Sprite();
	sprite3->initWithFile("Res/test.png");
	sprite3->setPosition(-512, -512);
	sprite3->setContentSize(512, 512);
	sprite3->setColor(0, 255, 0);
	sprite3->setOpacity(100);
	sprite3->autoRelease();
	sprite3->setLocalZ(0);
	sprite->addChild(sprite3);


	//sprite->autoRelease();
	/*Texture2D* texture = new Texture2D();
	texture->initWithFileName("Res/wall.jpg");
	texture->dump();
	texture->release();

	Texture2D* texture2 = new Texture2D();
	texture2->initWithFileName("Res/wall.jpg");
	texture2->dump();*/

	/*texture->initWithFileName("Res/wall.jpg", Image::IMAGE_RGB);
	texture->dump();*/
	/*Shader* s = new Shader();
	s->initWithFiles("Res\\Shaders\\default.vert", "Res\\Shaders\\default.frag");
	s->dump();*/

	//glCreateShader(GL_VERTEX_SHADER);
	/*Shader* shader = ShaderCache::getInstance()->getGlobalShader(ShaderCache::LX_SHADERS_PVM_DEFAULT);
	shader->dump();*/


	return true;

}

bool Game::loop()
{
	return true;
}


Game::~Game()
{
}