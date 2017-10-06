#include "stdafx.h"
#include "ParticleEmitter.h"
#include "Director.h"
#include "Common.h"


ParticleEmitter::ParticleEmitter():
_spriteFrame(nullptr),
_shader(nullptr),
_maxParticels(300),
_generateRate(60),
_particleLifespan(3.0f),
_particleLifespanVariance(0.25f),
_sourcePositionVariancex(40.0f),
_sourcePositionVariancey(20.0f),
_startParticleSize(54.0f),
_startParticleSizeVariance(10.0f),
_finishParticleSize(54.0f),
_finishParticleSizeVariance(0.0f),
_angle(90.0f),
_angleVariance(10.0f),
_speed(60.0f),
_speedVariance(20.0f),
_startColorRed(0.75f),
_startColorGreen(0.25f),
_startColorBlue(0.11f),
_startColorAlpha(1.0f),
_startColorVarianceRed(0.0f),
_startColorVarianceGreen(0.0f),
_startColorVarianceBlue(0.0f),
_startColorVarianceAlpha(0.0f),
_finishColorRed(0.0f),
_finishColorGreen(0.0f),
_finishColorBlue(0.0f),
_finishColorAlpha(1.0f),
_finishColorVarianceRed(0.0f),
_finishColorVarianceGreen(0.0f),
_finishColorVarianceBlue(0.0f),
_finishColorVarianceAlpha(0.0f),
_blendFuncDestination(1),
_blendFuncSource(770)
{
	setShader(ShaderCache::getInstance()->getGlobalShader(ShaderCache::LX_SHADERS_PARTICLE));
}

bool ParticleEmitter::initWithParticleInfo(unsigned int maxParticles, unsigned int generateRate, SpriteFrame* sp)
{
	_maxParticels = maxParticles;
	_generateRate = generateRate;
	setSpriteFrame(sp);
	return true;
}


ParticleEmitter::~ParticleEmitter()
{
	LX_SAFE_RELEASE_NULL(_shader);
	LX_SAFE_RELEASE_NULL(_spriteFrame);
	_particles.clear();
}

void ParticleEmitter::updateParticles()
{
	// Update existing particles
	float dt = Director::getInstance()->getDeltaTime();
	auto it = _particles.begin();
	while (it != _particles.end()) {
		it->_leftLifespan -= dt;
		if (it->_leftLifespan <= 0.0) {
			it = _particles.erase(it);
		}
		else {
			++it;
		}
	}
	// Create new particles
	int leftSize = _maxParticels - _particles.size();
	int newParticlesCnt = _generateRate * dt;

	int needCreateParticleSize;
	if (leftSize >= newParticlesCnt) {
		needCreateParticleSize = newParticlesCnt;
	}
	else if (leftSize > 0) {
		needCreateParticleSize = leftSize;
	}
	else {
		needCreateParticleSize = 0;
	}
	//LX_LOG("needCreateParticleSize %d\n" , needCreateParticleSize);
	glm::vec4 worldPos = _modelTransform * glm::vec4(0.0, 0.0, 0.0, 1.0);
	for (int i = 0; i < needCreateParticleSize; ++i) {
		// Initialize p
		Particle p;
		p._particleLifespan = _particleLifespan + RANDOM_MINUS_1_1() * _particleLifespanVariance;
		p._leftLifespan = p._particleLifespan;
		
		float angle = _angle + RANDOM_MINUS_1_1() * _angleVariance;
		float speed = _speed + RANDOM_MINUS_1_1() * _speedVariance;
		p._startPositionX = worldPos.x + RANDOM_MINUS_1_1() * _sourcePositionVariancex;
		p._finishPositionX = p._startPositionX + cosf(glm::radians(angle)) * speed * p._particleLifespan;
		p._startPositionY = worldPos.y + RANDOM_MINUS_1_1() * _sourcePositionVariancey;
		p._finishPositionY = p._startPositionY + sinf(glm::radians(angle)) * speed * p._particleLifespan;
		
		p._startParticleSize = _startParticleSize + RANDOM_MINUS_1_1() * _startParticleSizeVariance;
		p._finishParticleSize = _finishParticleSize + RANDOM_MINUS_1_1() * _finishParticleSizeVariance;
		

		p._startColorAlpha = _startColorAlpha + RANDOM_MINUS_1_1() * _startColorVarianceAlpha;
		p._startColorBlue = _startColorBlue + RANDOM_MINUS_1_1() * _startColorVarianceBlue;
		p._startColorGreen = _startColorGreen + RANDOM_MINUS_1_1() * _startColorVarianceGreen;
		p._startColorRed = _startColorRed + RANDOM_MINUS_1_1() * _startColorVarianceRed;
		
		p._finishColorAlpha = _finishColorAlpha + RANDOM_MINUS_1_1() * _finishColorVarianceAlpha;
		p._finishColorBlue = _finishColorBlue + RANDOM_MINUS_1_1() * _finishColorVarianceBlue;
		p._finishColorGreen = _finishColorGreen + RANDOM_MINUS_1_1() * _finishColorVarianceGreen;
		p._finishColorRed = _finishColorRed + RANDOM_MINUS_1_1() * _finishColorVarianceRed;
		
		_particles.push_back(p);
	}
}

void ParticleEmitter::visit(glm::mat4 & parentTransform)
{
	sortChildren();
	updateTransform();
	_modelTransform = parentTransform * _transform;
	// For particles, should update particles.
	updateParticles();
	updateAABB();
	auto start = _children.begin();
	// visit nodes with z < 0
	for (; start != _children.end(); ++start) {
		if ((*start)->getLocalZ() < 0) {
			(*start)->visit(_modelTransform);
		}
		else {
			break;
		}
	}
	draw();
	// visit nodes with z >= 0
	for (; start != _children.end(); ++start) {
		(*start)->visit(_modelTransform);
	}
}

void ParticleEmitter::setupVAOAndVBO()
{
	//Particle should recreate vao and vbo every frame
	fillPolygonInfo();

	//Upload vertex data
	if (_vao == 0) {
		glGenVertexArrays(1, &_vao);
		glGenBuffers(1, &_vbo);
		glGenBuffers(1, &_ebo);

		glBindVertexArray(_vao);
		glBindBuffer(GL_ARRAY_BUFFER, _vbo);
		glBufferData(GL_ARRAY_BUFFER, _polyInfo.getVertsCount() * sizeof(V3F_C4B_T2F), _polyInfo.getVerts(), GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, _polyInfo.getIndicesCount() * sizeof(GLuint), _polyInfo.getIndices(), GL_DYNAMIC_DRAW);

		//int a_position = _shader->getAttributeLocation("a_position");
		//int a_color = _shader->getAttributeLocation("a_color");
		//int a_texcoord = _shader->getAttributeLocation("a_texcoord");

		glVertexAttribPointer(_shader->getAttributeLocation("a_position"), 3, GL_FLOAT, GL_FALSE, sizeof(V3F_C4B_T2F), (GLvoid*)offsetof(V3F_C4B_T2F, _vertices));
		glEnableVertexAttribArray(_shader->getAttributeLocation("a_position"));
		glVertexAttribPointer(_shader->getAttributeLocation("a_color"), 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(V3F_C4B_T2F), (GLvoid*)offsetof(V3F_C4B_T2F, _color));
		glEnableVertexAttribArray(_shader->getAttributeLocation("a_color"));
		glVertexAttribPointer(_shader->getAttributeLocation("a_texcoord"), 2, GL_FLOAT, GL_FALSE, sizeof(V3F_C4B_T2F), (GLvoid*)offsetof(V3F_C4B_T2F, _uv));
		glEnableVertexAttribArray(_shader->getAttributeLocation("a_texcoord"));

		glBindVertexArray(0);
	}
	else {
		glBindVertexArray(_vao);

		glBindBuffer(GL_ARRAY_BUFFER, _vbo);
		glBufferData(GL_ARRAY_BUFFER, _polyInfo.getVertsCount() * sizeof(V3F_C4B_T2F), _polyInfo.getVerts(), GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, _polyInfo.getIndicesCount() * sizeof(GLuint), _polyInfo.getIndices(), GL_DYNAMIC_DRAW);

		glBindVertexArray(0);
	}
}

void ParticleEmitter::setShader(Shader * shader)
{
	if (_shader != nullptr) {
		_shader->release();
	}
	shader->retain();
	_shader = shader;
}

void ParticleEmitter::setSpriteFrame(SpriteFrame * spriteFrame)
{
	if (_spriteFrame != nullptr) {
		_spriteFrame->release();
	}
	spriteFrame->retain();
	_spriteFrame = spriteFrame;
}

void ParticleEmitter::fillPolygonInfo()
{
	_polyInfo.clear();
	const glm::vec2& size = _spriteFrame->getPixelRect().getSize();
	int count = 0;
	for (auto it : _particles) {
		V3F_C4B_T2F tmpLB, tmpRB, tmpRT, tmpLT;

		float currentScale = (it._particleLifespan - it._leftLifespan) / it._particleLifespan;

		float particleSize = LINEAR_INTERPOLATION(it._startParticleSize, it._finishParticleSize, currentScale);
		float particleRed = LINEAR_INTERPOLATION(it._startColorRed, it._finishColorRed, currentScale);
		float particleGreen = LINEAR_INTERPOLATION(it._startColorGreen, it._finishColorGreen, currentScale);
		float particleBlue = LINEAR_INTERPOLATION(it._startColorBlue, it._finishColorBlue, currentScale);
		float particleAlpha = LINEAR_INTERPOLATION(it._startColorAlpha, it._finishColorAlpha, currentScale);
		float particleWorldPosX = LINEAR_INTERPOLATION(it._startPositionX, it._finishPositionX, currentScale);
		float particleWorldPosY = LINEAR_INTERPOLATION(it._startPositionY, it._finishPositionY, currentScale);
		
		float halfParticleSize = particleSize / 2.0;
		glm::vec2 lbPos = glm::vec2(particleWorldPosX - halfParticleSize, particleWorldPosY - halfParticleSize);
		glm::vec2 rbPos = glm::vec2(particleWorldPosX + halfParticleSize, particleWorldPosY - halfParticleSize);
		glm::vec2 rtPos = glm::vec2(particleWorldPosX + halfParticleSize, particleWorldPosY + halfParticleSize);
		glm::vec2 ltPos = glm::vec2(particleWorldPosX - halfParticleSize, particleWorldPosY + halfParticleSize);

		tmpLB.setVertices(lbPos.x, lbPos.y, 0.0);
		tmpRB.setVertices(rbPos.x, rbPos.y, 0.0);
		tmpRT.setVertices(rtPos.x, rtPos.y, 0.0);
		tmpLT.setVertices(ltPos.x, ltPos.y, 0.0);

		GLubyte red = GLubyte(particleRed * 255);
		GLubyte green = GLubyte(particleGreen * 255);
		GLubyte blue = GLubyte(particleBlue * 255);
		GLubyte alpha = GLubyte(particleAlpha * 255);

		tmpLB.setColor(red, green, blue, alpha);
		tmpRB.setColor(red, green, blue, alpha);
		tmpRT.setColor(red, green, blue, alpha);
		tmpLT.setColor(red, green, blue, alpha);

		tmpLB.setUV(_spriteFrame->getLBTexCoord().x, _spriteFrame->getLBTexCoord().y);
		tmpRB.setUV(_spriteFrame->getRBTexCoord().x, _spriteFrame->getRBTexCoord().y);
		tmpRT.setUV(_spriteFrame->getRTTexCoord().x, _spriteFrame->getRTTexCoord().y);
		tmpLT.setUV(_spriteFrame->getLTTexCoord().x, _spriteFrame->getLTTexCoord().y);

		_polyInfo.pushVert(tmpLB);
		_polyInfo.pushVert(tmpRB);
		_polyInfo.pushVert(tmpRT);
		_polyInfo.pushVert(tmpLT);

		// 013123
		_polyInfo.pushIndices(count * 4);
		_polyInfo.pushIndices(count * 4 + 1);
		_polyInfo.pushIndices(count * 4 + 3);

		_polyInfo.pushIndices(count * 4 + 1);
		_polyInfo.pushIndices(count * 4 + 2);
		_polyInfo.pushIndices(count * 4 + 3);
		++count;
	}
	
}

void ParticleEmitter::draw()
{
	setupVAOAndVBO();
	_shader->use();
	_shader->updateBuiltinUniforms(glm::mat4());

	/*auto instance = Camera::getInstance();
	glm::mat4 temp = instance->getProjectionMatrix() * instance->getViewMatrix() * _modelTransform;
	std::vector<V3F_C4B_T2F>* vertices = static_cast<std::vector<V3F_C4B_T2F>*>(_polyInfo.getVerts());
	for (auto it : *vertices) {
	auto xx = it._vertices[0];
	auto tt = it._vertices[1];
	auto zz = it._vertices[2];
	glm::vec4 pos = glm::vec4(xx, tt, zz, 1.0);
	glm::vec4 finalPos = temp * pos;
	}*/

	glActiveTexture(GL_TEXTURE0); //在绑定纹理之前先激活纹理单元
	_spriteFrame->bindTexture();
	glBlendFunc(_blendFuncSource, _blendFuncDestination);
	glBindVertexArray(_vao);
	glDrawElements(GL_TRIANGLES, _polyInfo.getIndicesCount(), GL_UNSIGNED_INT, 0);
	//LX_LOG("%d\n", _polyInfo.getIndicesCount());
	glBindVertexArray(0);
}

