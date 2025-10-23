///////////////////////////////////////////////////////////////////////////////
// SceneManager.cpp
// ============
// Manages the loading, setup, and rendering of 3D scenes with lighting,
// textures, materials, and shadow mapping for OpenGL applications.
//
// AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
// Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>
#include <algorithm> // For std::sort
#include <ranges>

// Global shader uniform variable names used throughout the application
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
	const int SHADOW_MAP_SIZE = 1024; // Shadow map texture resolution
}

// Scene configuration constants to prevent errors and improve maintainability
namespace SceneConstants
{
	// Texture identifiers - prevents typos and makes maintenance easier
	const std::string DESK_TEXTURE = "deskTexture";
	const std::string LAPTOP_TEXTURE = "laptopTex";
	const std::string SCREEN_TEXTURE = "screenTex";
	const std::string MUG_TEXTURE = "mugTex";
	const std::string HANDLE_TEXTURE = "handleTex";
	const std::string FLOOR_TEXTURE = "floorTex";
	const std::string WALL_TEXTURE = "wallTex";
	const std::string LAMP_SHADE_TEXTURE = "lampShadeTex";
	const std::string LAMP_BASE_TEXTURE = "lampBaseTex";
	
	// File paths and directory constants
	const std::string TEXTURE_DIR = "../Utilities/textures/";
	const std::string DESK_TEXTURE_FILE = "rusticwood.jpg";
	const std::string LAPTOP_TEXTURE_FILE = "stainless.jpg";
	const std::string SCREEN_TEXTURE_FILE = "wallpaper.jpg";
	const std::string MUG_TEXTURE_FILE = "tilesf2.jpg";
	const std::string HANDLE_TEXTURE_FILE = "stainedglass.jpg";
	const std::string FLOOR_TEXTURE_FILE = "pavers.jpg";
	const std::string WALL_TEXTURE_FILE = "backdrop.jpg";
	const std::string LAMP_SHADE_TEXTURE_FILE = "circular-brushed-gold-texture.jpg";
	const std::string LAMP_BASE_TEXTURE_FILE = "stainless_end.jpg";
	
	// Shader file paths
	const std::string DEPTH_VERTEX_SHADER = "../Utilities/shaders/depthVertexShader.glsl";
	const std::string DEPTH_FRAGMENT_SHADER = "../Utilities/shaders/depthFragmentShader.glsl";
	
	// Camera and lighting positions
	const glm::vec3 CAMERA_POSITION = glm::vec3(0.0f, 5.0f, 15.0f);
	const glm::vec3 DRAMATIC_LIGHT_POSITION = glm::vec3(2.0f, 8.0f, -3.0f);
	const glm::vec3 DRAMATIC_LIGHT_DIRECTION = glm::vec3(-0.3f, -1.0f, 0.4f);
	const glm::vec3 BALANCED_LIGHT_POSITION = glm::vec3(1.0f, 12.0f, 2.0f);
	const glm::vec3 BALANCED_LIGHT_DIRECTION = glm::vec3(-0.1f, -1.0f, -0.2f);
	
	// Second colored light constants
	const glm::vec3 ACCENT_LIGHT_POSITION = glm::vec3(-5.0f, 6.0f, 8.0f);
	const glm::vec3 ACCENT_LIGHT_AMBIENT = glm::vec3(0.1f, 0.1f, 0.3f);
	const glm::vec3 ACCENT_LIGHT_DIFFUSE = glm::vec3(0.3f, 0.5f, 1.0f);
	const glm::vec3 ACCENT_LIGHT_SPECULAR = glm::vec3(0.5f, 0.7f, 1.0f);
	
	// Object transformations - Desk
	const glm::vec3 DESK_SCALE = glm::vec3(38.0f, 0.475f, 23.75f);
	const glm::vec3 DESK_POSITION = glm::vec3(0.0f, -0.5f, -5.0f);
	const glm::vec3 DESK_ROTATION = glm::vec3(0.0f, 0.0f, 0.0f);
	
	// Object transformations - Coffee Mug
	const glm::vec3 MUG_BODY_SCALE = glm::vec3(0.575f, 1.725f, 0.575f);
	const glm::vec3 MUG_BODY_POSITION = glm::vec3(5.0f, 0.75f, -2.0f);
	const glm::vec3 MUG_BODY_ROTATION = glm::vec3(15.0f, 20.0f, 0.0f);
	
	const glm::vec3 MUG_HANDLE_SCALE = glm::vec3(0.36225f, 0.36225f, 0.36225f);
	const glm::vec3 MUG_HANDLE_POSITION = glm::vec3(5.5f, 1.5f, -2.0f);
	const glm::vec3 MUG_HANDLE_ROTATION = glm::vec3(0.0f, 0.0f, 90.0f);
	
	// Object transformations - Laptop
	const glm::vec3 LAPTOP_BASE_SCALE = glm::vec3(6.6f, 0.11f, 4.4f);
	const glm::vec3 LAPTOP_BASE_POSITION = glm::vec3(0.0f, 0.88f, -0.55f);
	const glm::vec3 LAPTOP_BASE_ROTATION = glm::vec3(0.0f, 0.0f, 0.0f);
	
	const glm::vec3 LAPTOP_SCREEN_SCALE = glm::vec3(6.6f, 3.3f, 0.11f);
	const glm::vec3 LAPTOP_SCREEN_POSITION = glm::vec3(0.0f, 1.76f, -2.2f);
	const glm::vec3 LAPTOP_SCREEN_ROTATION = glm::vec3(-45.0f, 0.0f, 0.0f);
	
	const glm::vec3 DISPLAY_PANEL_SCALE = glm::vec3(6.05f, 3.08f, 0.055f);
	const glm::vec3 DISPLAY_PANEL_POSITION = glm::vec3(0.0f, 1.76f, -2.145f);
	const glm::vec3 DISPLAY_PANEL_ROTATION = glm::vec3(-45.0f, 0.0f, 0.0f);
	
	// Object transformations - Desk Lamp (PROPERLY FIXED - uses Cone and Cylinder)
	const glm::vec3 LAMP_BASE_SCALE = glm::vec3(1.0f, 0.4f, 1.0f);
	const glm::vec3 LAMP_BASE_POSITION = glm::vec3(-6.0f, 0.2f, 2.0f);
	const glm::vec3 LAMP_BASE_ROTATION = glm::vec3(0.0f, 0.0f, 0.0f);
	
	const glm::vec3 LAMP_SHADE_SCALE = glm::vec3(1.4f, 1.2f, 1.4f);
	const glm::vec3 LAMP_SHADE_POSITION = glm::vec3(-6.0f, 2.5f, 2.0f);
	const glm::vec3 LAMP_SHADE_ROTATION = glm::vec3(0.0f, 0.0f, 0.0f);

	const glm::vec3 LAMP_STEM_SCALE = glm::vec3(0.12f, 2.8f, 0.12f);     
	const glm::vec3 LAMP_STEM_POSITION = glm::vec3(-6.0f, 0.5f, 2.0f);   
	const glm::vec3 LAMP_STEM_ROTATION = glm::vec3(0.0f, 0.0f, 0.0f);
	
	// Object transformations - Floor and Wall
	const glm::vec3 FLOOR_SCALE = glm::vec3(62.4f, 0.13f, 32.5f);
	const glm::vec3 FLOOR_POSITION = glm::vec3(0.0f, -5.0f, -5.0f);
	const glm::vec3 FLOOR_ROTATION = glm::vec3(0.0f, 0.0f, 0.0f);
	
	const glm::vec3 WALL_SCALE = glm::vec3(62.4f, 26.0f, 0.65f);
	const glm::vec3 WALL_POSITION_OFFSET = glm::vec3(0.0f, 0.0f, -21.25f);
	const glm::vec3 WALL_ROTATION = glm::vec3(0.0f, 0.0f, 0.0f);
	
	// Keyboard key constants
	const float KEY_WIDTH = 0.385f;
	const float KEY_HEIGHT = 0.11f;
	const float KEY_DEPTH = 0.33f;
	const int KEY_ROWS = 5;
	const int KEY_COLS = 12;
	const float KEY_SPACING = 0.055f;
	const float KEY_START_X = -2.31f;
	const float KEY_Y = 0.902f;
	const float KEY_START_Z = -1.1f;
	const glm::vec3 KEY_COLOR = glm::vec3(0.15f, 0.15f, 0.15f);
	
	// Material property constants
	const float DRAMATIC_AMBIENT_STRENGTH = 0.05f;
	const float METAL_AMBIENT_STRENGTH = 0.03f;
	const float STANDARD_AMBIENT_STRENGTH = 0.05f;
	const float WOOD_SHININESS = 32.0f;
	const float CERAMIC_SHININESS = 128.0f;
	const float METAL_SHININESS = 256.0f;
	const float STONE_SHININESS = 64.0f;
	
	// Material color constants
	const glm::vec3 WOOD_AMBIENT_COLOR = glm::vec3(1.0f, 0.9f, 0.7f);
	const glm::vec3 WOOD_DIFFUSE_COLOR = glm::vec3(1.0f, 0.9f, 0.6f);
	const glm::vec3 WOOD_SPECULAR_COLOR = glm::vec3(0.5f, 0.5f, 0.5f);
	
	const glm::vec3 CERAMIC_AMBIENT_COLOR = glm::vec3(0.9f, 0.9f, 1.0f);
	const glm::vec3 CERAMIC_DIFFUSE_COLOR = glm::vec3(0.8f, 0.8f, 0.9f);
	const glm::vec3 CERAMIC_SPECULAR_COLOR = glm::vec3(1.0f, 1.0f, 1.0f);
	
	const glm::vec3 METAL_AMBIENT_COLOR = glm::vec3(0.3f, 0.3f, 0.3f);
	const glm::vec3 METAL_DIFFUSE_COLOR = glm::vec3(0.4f, 0.4f, 0.4f);
	const glm::vec3 METAL_SPECULAR_COLOR = glm::vec3(0.7f, 0.7f, 0.7f);
	
	const glm::vec3 STONE_AMBIENT_COLOR = glm::vec3(0.3f, 0.3f, 0.3f);
	const glm::vec3 STONE_DIFFUSE_COLOR = glm::vec3(0.6f, 0.6f, 0.6f);
	const glm::vec3 STONE_SPECULAR_COLOR = glm::vec3(0.3f, 0.3f, 0.3f);
	
	const glm::vec3 LAMP_SHADE_COLOR = glm::vec3(0.9f, 0.9f, 0.8f);
	const glm::vec3 DEFAULT_COLOR = glm::vec3(1.0f, 1.0f, 1.0f);
	
	// Lighting constants
	const float DRAMATIC_LIGHT_INTENSITY = 25.0f;
	const float BALANCED_AMBIENT_INTENSITY = 0.4f;
	const float BALANCED_DIFFUSE_INTENSITY = 8.0f;
	const float BALANCED_SPECULAR_INTENSITY = 5.0f;
	const float TIGHT_SPOTLIGHT_INNER = 15.0f; // degrees
	const float TIGHT_SPOTLIGHT_OUTER = 25.0f; // degrees
	const float BALANCED_SPOTLIGHT_INNER = 25.0f; // degrees
	const float BALANCED_SPOTLIGHT_OUTER = 45.0f; // degrees
	
	// UV scaling constants
	const glm::vec2 DEFAULT_UV_SCALE = glm::vec2(1.0f, 1.0f);
	const glm::vec2 LAPTOP_UV_SCALE = glm::vec2(2.0f, 2.0f);
	const glm::vec2 WALL_UV_SCALE = glm::vec2(2.0f, 2.0f);
	const glm::vec2 FLOOR_UV_SCALE = glm::vec2(4.0f, 4.0f);
	
	// Wall positioning constants
	const float FLOOR_Y = -5.0f;
	const float FLOOR_HEIGHT = 0.13f;
	
	// Debug messages
	const std::string DEBUG_RENDER_START = "=== RenderScene called ===";
	const std::string DEBUG_RENDER_END = "=== RenderScene completed ===";
	const std::string DEBUG_DRAWING_OBJECTS = "About to draw objects...";
	const std::string DEBUG_LIGHTING_SETUP = "DEBUG: About to draw with lighting uniforms...";
	const std::string DEBUG_LIGHTS_SET = "DEBUG: Set numActiveLights to 2";
	const std::string DEBUG_LIGHT_POSITION = "DEBUG: Set light positions - primary and accent";
}

/***********************************************************
 *  SceneManager()
 *
 *  Constructor for the SceneManager class. Initializes the
 *  shader manager pointer, allocates basic mesh storage,
 *  and resets all texture slots to empty state for loading.
 ***********************************************************/
SceneManager::SceneManager(ShaderManager *pShaderManager)
{
    m_pShaderManager = pShaderManager;
    m_basicMeshes = new ShapeMeshes();

    // Initialize all 16 texture slots to empty state
    for (int i = 0; i < 16; i++)
    {
        m_textureIDs[i].tag = "/0";
        m_textureIDs[i].ID = -1;
    }
    m_loadedTextures = 0;
}

/***********************************************************
 *  ~SceneManager()
 *
 *  Destructor for the SceneManager class. Properly cleans up
 *  allocated memory and nullifies pointers to prevent memory
 *  leaks and dangling pointer access.
 ***********************************************************/
SceneManager::~SceneManager()
{
	m_pShaderManager = nullptr;
	delete m_basicMeshes;
	m_basicMeshes = nullptr;
}

/***********************************************************
 *  CreateGLTexture()
 *
 *  Loads an image file using STB library, creates an OpenGL
 *  texture object with proper parameters, and stores the
 *  texture information for later use. Supports RGB and RGBA
 *  image formats with automatic mipmap generation.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// Configure STB to flip images vertically to match OpenGL coordinate system
	stbi_set_flip_vertically_on_load(true);

	// Load image data from file system
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// Process successfully loaded image
	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height << ", channels:" << colorChannels << std::endl;

		// Generate and bind OpenGL texture object
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// Configure texture wrapping and filtering for optimal quality
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Upload image data to GPU based on channel count
		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		else if (colorChannels == 4)
		 glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Unsupported image format with " << colorChannels << " channels" << std::endl;
			stbi_image_free(image);
			return false;
		}

		// Generate mipmaps for improved rendering quality at distance
		glGenerateMipmap(GL_TEXTURE_2D);

		// Clean up resources and unbind texture
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0);

		// Store texture metadata for scene management
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Failed to load image:" << filename << std::endl;
	return false;
}

/***********************************************************
 *  BindGLTextures()
 *
 *  Activates texture units 0 through N-1 where N is the 
 *  number of loaded textures, and binds each texture to its
 *  respective unit. Enables shaders to access multiple
 *  textures simultaneously for complex rendering.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  Properly deallocates all OpenGL texture resources from
 *  GPU memory and resets internal tracking structures to
 *  prevent resource leaks during application shutdown.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glDeleteTextures(1, &m_textureIDs[i].ID);
		m_textureIDs[i].ID = -1;
		m_textureIDs[i].tag = "/0";
	}
	m_loadedTextures = 0;
}

/***********************************************************
 *  FindTextureID()
 *
 *  Performs linear search through loaded textures to find
 *  the OpenGL texture ID matching the provided tag. Used
 *  for binding specific textures during rendering operations.
 *  Returns -1 if texture not found.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  Searches for a texture by tag and returns its array
 *  index position. Useful for texture unit binding
 *  operations in multi-texture scenarios. Returns -1
 *  if texture tag is not found.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
		 index++;
	}

	return(textureSlot);
}

/***********************************************************
 *  FindMaterial()
 *
 *  Searches the materials collection for a material
 *  matching the provided tag and copies its properties
 *  (ambient, diffuse, specular colors, and shininess)
 *  into the provided material reference. Returns true
 *  if material found and populated successfully.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return(false);
	}

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.ambientColor = m_objectMaterials[index].ambientColor;
			material.ambientStrength = m_objectMaterials[index].ambientStrength;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return(true);
}

/***********************************************************
 *  SetTransformations()
 *
 *  Constructs a 4x4 transformation matrix using standard
 *  TRS order (Translation * Rotation * Scale) and passes
 *  it to the shader. Transformations are applied in the
 *  order: Scale -> Rotate X -> Rotate Y -> Rotate Z -> Translate.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// Create individual transformation matrices
	scale = glm::scale(scaleXYZ);
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	translation = glm::translate(positionXYZ);

	// Combine transformations in proper order: T * Rx * Ry * Rz * S
	modelView = translation * rotationX * rotationY * rotationZ * scale;

	// Pass transformation matrix to shader
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 *  SetShaderColor()
 *
 *  Configures the shader to use a solid color instead of
 *  texture mapping. Sets the color uniform and disables
 *  texture usage. Useful for debugging, wireframe rendering,
 *  or simple colored objects like keyboard keys.
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	glm::vec4 currentColor;
	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  Locates a texture by tag, activates its texture unit,
 *  binds the texture, and configures shader uniforms for
 *  texture-based rendering. Enables multi-texture support
 *  through proper texture unit management.
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);

		int slot = FindTextureSlot(textureTag);
		if (slot >= 0)
		{
			glActiveTexture(GL_TEXTURE0 + slot);
			glBindTexture(GL_TEXTURE_2D, m_textureIDs[slot].ID);
			m_pShaderManager->setSampler2DValue(g_TextureValueName, slot);
		}
	}
}

/***********************************************************
 *  SetTextureUVScale()
 *
 *  Controls texture tiling by scaling UV coordinates.
 *  Values greater than 1.0 create tiled textures across
 *  the surface, while values less than 1.0 stretch the
 *  texture. Used for floor tiling and texture repetition.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  Configures physically-based material properties for
 *  dramatic lighting effects. Each material is optimized
 *  for specific surface types (wood, metal, ceramic, stone)
 *  with carefully tuned ambient, diffuse, specular colors
 *  and shininess values for enhanced visual realism.
 ***********************************************************/
void SceneManager::SetShaderMaterial(std::string materialTag)
{
    if (materialTag == SceneConstants::DESK_TEXTURE)
    {
        // Wood surface - warm tones with moderate reflectivity
        m_pShaderManager->setVec3Value("material.ambientColor", SceneConstants::WOOD_AMBIENT_COLOR);
        m_pShaderManager->setFloatValue("material.ambientStrength", SceneConstants::DRAMATIC_AMBIENT_STRENGTH);
        m_pShaderManager->setVec3Value("material.diffuseColor", SceneConstants::WOOD_DIFFUSE_COLOR);
        m_pShaderManager->setVec3Value("material.specularColor", SceneConstants::WOOD_SPECULAR_COLOR);
        m_pShaderManager->setFloatValue("material.shininess", SceneConstants::WOOD_SHININESS);
    }
    else if (materialTag == SceneConstants::MUG_TEXTURE)
    {
        // Ceramic surface - cool tones with high reflectivity for shine
        m_pShaderManager->setVec3Value("material.ambientColor", SceneConstants::CERAMIC_AMBIENT_COLOR);
        m_pShaderManager->setFloatValue("material.ambientStrength", SceneConstants::DRAMATIC_AMBIENT_STRENGTH);
        m_pShaderManager->setVec3Value("material.diffuseColor", SceneConstants::CERAMIC_DIFFUSE_COLOR);
        m_pShaderManager->setVec3Value("material.specularColor", SceneConstants::CERAMIC_SPECULAR_COLOR);
        m_pShaderManager->setFloatValue("material.shininess", SceneConstants::CERAMIC_SHININESS);
    }
    else if (materialTag == SceneConstants::LAPTOP_TEXTURE)
    {
        // Metal surface - neutral tones with very high shininess
        m_pShaderManager->setVec3Value("material.ambientColor", SceneConstants::METAL_AMBIENT_COLOR);
        m_pShaderManager->setFloatValue("material.ambientStrength", SceneConstants::METAL_AMBIENT_STRENGTH);
        m_pShaderManager->setVec3Value("material.diffuseColor", SceneConstants::METAL_DIFFUSE_COLOR);
        m_pShaderManager->setVec3Value("material.specularColor", SceneConstants::METAL_SPECULAR_COLOR);
        m_pShaderManager->setFloatValue("material.shininess", SceneConstants::METAL_SHININESS);
    }
    else if (materialTag == SceneConstants::FLOOR_TEXTURE)
    {
        // Stone/concrete surface - neutral with low reflectivity
        m_pShaderManager->setVec3Value("material.ambientColor", SceneConstants::STONE_AMBIENT_COLOR);
        m_pShaderManager->setFloatValue("material.ambientStrength", SceneConstants::METAL_AMBIENT_STRENGTH);
        m_pShaderManager->setVec3Value("material.diffuseColor", SceneConstants::STONE_DIFFUSE_COLOR);
        m_pShaderManager->setVec3Value("material.specularColor", SceneConstants::STONE_SPECULAR_COLOR);
        m_pShaderManager->setFloatValue("material.shininess", SceneConstants::STONE_SHININESS);
    }
    else if (materialTag == SceneConstants::WALL_TEXTURE)
    {
        // Wall surface - matte finish with minimal reflectivity
        m_pShaderManager->setVec3Value("material.ambientColor", SceneConstants::STONE_AMBIENT_COLOR);
        m_pShaderManager->setFloatValue("material.ambientStrength", SceneConstants::METAL_AMBIENT_STRENGTH);
        m_pShaderManager->setVec3Value("material.diffuseColor", SceneConstants::STONE_DIFFUSE_COLOR);
        m_pShaderManager->setVec3Value("material.specularColor", SceneConstants::STONE_SPECULAR_COLOR);
        m_pShaderManager->setFloatValue("material.shininess", SceneConstants::STONE_SHININESS);
    }
    else if (materialTag == SceneConstants::LAMP_SHADE_TEXTURE)
    {
        // Lamp shade material - golden metallic finish
        m_pShaderManager->setVec3Value("material.ambientColor", glm::vec3(1.0f, 0.8f, 0.4f));
        m_pShaderManager->setFloatValue("material.ambientStrength", 0.3f);
        m_pShaderManager->setVec3Value("material.diffuseColor", glm::vec3(0.9f, 0.7f, 0.3f));
        m_pShaderManager->setVec3Value("material.specularColor", glm::vec3(1.0f, 0.9f, 0.6f));
        m_pShaderManager->setFloatValue("material.shininess", SceneConstants::METAL_SHININESS);
    }
    else if (materialTag == SceneConstants::LAMP_BASE_TEXTURE)
    {
        // Lamp base material - stainless steel finish
        m_pShaderManager->setVec3Value("material.ambientColor", SceneConstants::METAL_AMBIENT_COLOR);
        m_pShaderManager->setFloatValue("material.ambientStrength", SceneConstants::METAL_AMBIENT_STRENGTH);
        m_pShaderManager->setVec3Value("material.diffuseColor", SceneConstants::METAL_DIFFUSE_COLOR);
        m_pShaderManager->setVec3Value("material.specularColor", SceneConstants::METAL_SPECULAR_COLOR);
        m_pShaderManager->setFloatValue("material.shininess", SceneConstants::METAL_SHININESS);
    }
    else
    {
        // Default material - neutral properties for unspecified materials
        m_pShaderManager->setVec3Value("material.ambientColor", SceneConstants::DEFAULT_COLOR);
        m_pShaderManager->setFloatValue("material.ambientStrength", SceneConstants::STANDARD_AMBIENT_STRENGTH);
        m_pShaderManager->setVec3Value("material.diffuseColor", SceneConstants::DEFAULT_COLOR);
        m_pShaderManager->setVec3Value("material.specularColor", SceneConstants::DEFAULT_COLOR);
        m_pShaderManager->setFloatValue("material.shininess", SceneConstants::WOOD_SHININESS);
    }
}

/***********************************************************
 *  SetModelMatrix()
 *
 *  Constructs transformation matrix and applies it to both
 *  main shader and depth shader for shadow mapping consistency.
 *  Used during shadow pass where the same transformation
 *  must be applied to both rendering passes.
 ***********************************************************/
void SceneManager::SetModelMatrix(glm::vec3 scaleXYZ, float Xrot, float Yrot, float Zrot, glm::vec3 positionXYZ)
{
	// Construct individual transformation components
	glm::mat4 scale = glm::scale(scaleXYZ);
	glm::mat4 rotX = glm::rotate(glm::radians(Xrot), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 rotY = glm::rotate(glm::radians(Yrot), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 rotZ = glm::rotate(glm::radians(Zrot), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 translation = glm::translate(positionXYZ);

	// Combine transformations in standard order
	glm::mat4 model = translation * rotX * rotY * rotZ * scale;

	// Apply to both main and depth shaders for shadow consistency
	if (m_pShaderManager)
		m_pShaderManager->setMat4Value("model", model);

	if (m_pDepthShaderManager)
		m_pDepthShaderManager->setMat4Value("model", model);
}

/***********************************************************
 *  PrepareScene()
 *
 *  Comprehensive scene initialization including mesh loading,
 *  texture creation with error handling, material property
 *  configuration, dual-light setup with colored accent light,
 *  and shadow mapping initialization. Sets up the complete 
 *  rendering pipeline for the 3D scene.
 ***********************************************************/
void SceneManager::PrepareScene()
{
	// Initialize depth shader manager for shadow mapping using constants
	m_pDepthShaderManager = new ShaderManager();
	m_pDepthShaderManager->LoadShaders(
		SceneConstants::DEPTH_VERTEX_SHADER.c_str(),
		SceneConstants::DEPTH_FRAGMENT_SHADER.c_str());

	// Load all basic geometric meshes for scene objects including new cone
	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadCylinderMesh();
	m_basicMeshes->LoadTorusMesh();
	m_basicMeshes->LoadBoxMesh();
	m_basicMeshes->LoadConeMesh(); // NEW: Load cone mesh for lamp shade

	// Load texture assets using constants to prevent filename errors
	if (!CreateGLTexture((SceneConstants::TEXTURE_DIR + SceneConstants::DESK_TEXTURE_FILE).c_str(), SceneConstants::DESK_TEXTURE))
		std::cerr << "Failed to load desk texture!" << std::endl;
	if (!CreateGLTexture((SceneConstants::TEXTURE_DIR + SceneConstants::LAPTOP_TEXTURE_FILE).c_str(), SceneConstants::LAPTOP_TEXTURE))
		std::cerr << "Failed to load laptop texture!" << std::endl;
	if (!CreateGLTexture((SceneConstants::TEXTURE_DIR + SceneConstants::SCREEN_TEXTURE_FILE).c_str(), SceneConstants::SCREEN_TEXTURE))
		std::cerr << "Failed to load screen texture!" << std::endl;
	if (!CreateGLTexture((SceneConstants::TEXTURE_DIR + SceneConstants::MUG_TEXTURE_FILE).c_str(), SceneConstants::MUG_TEXTURE))
		std::cerr << "Failed to load mug texture!" << std::endl;
	if (!CreateGLTexture((SceneConstants::TEXTURE_DIR + SceneConstants::HANDLE_TEXTURE_FILE).c_str(), SceneConstants::HANDLE_TEXTURE))
		std::cerr << "Failed to load handle texture!" << std::endl;
	if (!CreateGLTexture((SceneConstants::TEXTURE_DIR + SceneConstants::FLOOR_TEXTURE_FILE).c_str(), SceneConstants::FLOOR_TEXTURE))
		std::cerr << "Failed to load floor texture!" << std::endl;
	if (!CreateGLTexture((SceneConstants::TEXTURE_DIR + SceneConstants::WALL_TEXTURE_FILE).c_str(), SceneConstants::WALL_TEXTURE))
		std::cerr << "Failed to load wall texture!" << std::endl;
	if (!CreateGLTexture((SceneConstants::TEXTURE_DIR + SceneConstants::LAMP_SHADE_TEXTURE_FILE).c_str(), SceneConstants::LAMP_SHADE_TEXTURE))
		std::cerr << "Failed to load lamp shade texture!" << std::endl;
	if (!CreateGLTexture((SceneConstants::TEXTURE_DIR + SceneConstants::LAMP_BASE_TEXTURE_FILE).c_str(), SceneConstants::LAMP_BASE_TEXTURE))
		std::cerr << "Failed to load lamp base texture!" << std::endl;

	// Configure material properties using constants for consistency
	OBJECT_MATERIAL deskMaterial;
	deskMaterial.tag = SceneConstants::DESK_TEXTURE;
	deskMaterial.ambientColor = SceneConstants::WOOD_AMBIENT_COLOR;
	deskMaterial.ambientStrength = 0.3f;
	deskMaterial.diffuseColor = SceneConstants::WOOD_DIFFUSE_COLOR;
	deskMaterial.specularColor = SceneConstants::WOOD_SPECULAR_COLOR;
	deskMaterial.shininess = SceneConstants::WOOD_SHININESS;
	m_objectMaterials.push_back(deskMaterial);

	OBJECT_MATERIAL laptopMaterial;
	laptopMaterial.tag = SceneConstants::LAPTOP_TEXTURE;
	laptopMaterial.ambientColor = SceneConstants::METAL_AMBIENT_COLOR;
	laptopMaterial.ambientStrength = 0.2f;
	laptopMaterial.diffuseColor = SceneConstants::METAL_DIFFUSE_COLOR;
	laptopMaterial.specularColor = SceneConstants::METAL_SPECULAR_COLOR;
	laptopMaterial.shininess = SceneConstants::METAL_SHININESS;
	m_objectMaterials.push_back(laptopMaterial);

	OBJECT_MATERIAL mugMaterial;
	mugMaterial.tag = SceneConstants::MUG_TEXTURE;
	mugMaterial.ambientColor = SceneConstants::CERAMIC_AMBIENT_COLOR;
	mugMaterial.ambientStrength = 0.3f;
	mugMaterial.diffuseColor = SceneConstants::CERAMIC_DIFFUSE_COLOR;
	mugMaterial.specularColor = glm::vec3(0.8f, 0.8f, 0.8f);
	mugMaterial.shininess = SceneConstants::CERAMIC_SHININESS;
	m_objectMaterials.push_back(mugMaterial);

	OBJECT_MATERIAL floorMaterial;
	floorMaterial.tag = SceneConstants::FLOOR_TEXTURE;
	floorMaterial.ambientColor = SceneConstants::STONE_AMBIENT_COLOR;
	floorMaterial.ambientStrength = 0.2f;
	floorMaterial.diffuseColor = SceneConstants::STONE_DIFFUSE_COLOR;
	floorMaterial.specularColor = SceneConstants::STONE_SPECULAR_COLOR;
	floorMaterial.shininess = SceneConstants::STONE_SHININESS;
	m_objectMaterials.push_back(floorMaterial);

	OBJECT_MATERIAL wallMaterial;
	wallMaterial.tag = SceneConstants::WALL_TEXTURE;
	wallMaterial.ambientColor = SceneConstants::STONE_AMBIENT_COLOR;
	wallMaterial.ambientStrength = 0.2f;
	wallMaterial.diffuseColor = SceneConstants::STONE_DIFFUSE_COLOR;
	wallMaterial.specularColor = SceneConstants::STONE_SPECULAR_COLOR;
	wallMaterial.shininess = SceneConstants::STONE_SHININESS;
	m_objectMaterials.push_back(wallMaterial);

	// NEW: Add lamp shade material
	OBJECT_MATERIAL lampShadeMaterial;
	lampShadeMaterial.tag = SceneConstants::LAMP_SHADE_TEXTURE;
	lampShadeMaterial.ambientColor = glm::vec3(1.0f, 0.8f, 0.4f); // Golden tint
	lampShadeMaterial.ambientStrength = 0.3f;
	lampShadeMaterial.diffuseColor = glm::vec3(0.9f, 0.7f, 0.3f); // Golden diffuse
	lampShadeMaterial.specularColor = glm::vec3(1.0f, 0.9f, 0.6f); // Golden specular
	lampShadeMaterial.shininess = SceneConstants::METAL_SHININESS;
	m_objectMaterials.push_back(lampShadeMaterial);

	// NEW: Add lamp base material
	OBJECT_MATERIAL lampBaseMaterial;
	lampBaseMaterial.tag = SceneConstants::LAMP_BASE_TEXTURE;
	lampBaseMaterial.ambientColor = SceneConstants::METAL_AMBIENT_COLOR;
	lampBaseMaterial.ambientStrength = 0.2f;
	lampBaseMaterial.diffuseColor = SceneConstants::METAL_DIFFUSE_COLOR;
	lampBaseMaterial.specularColor = SceneConstants::METAL_SPECULAR_COLOR;
	lampBaseMaterial.shininess = SceneConstants::METAL_SHININESS;
	m_objectMaterials.push_back(lampBaseMaterial);

	// Bind all loaded textures to their respective texture units
	BindGLTextures();

	// Configure camera position using constants
	m_pShaderManager->setVec3Value("viewPosition", SceneConstants::CAMERA_POSITION);

	// Setup dual-light configuration with primary spotlight and colored accent light
	const int MAX_ACTIVE_LIGHTS = 8;
	const float LIGHT_EFFECT_RADIUS = 20.0f;

	struct LightSetup {
		int index;
		glm::vec3 position;
		glm::vec3 ambientColor;
		glm::vec3 diffuseColor;
		glm::vec3 specularColor;
		float focalStrength;
		float specularIntensity;
		glm::vec3 spotDirection;
		float cutoff;
		float outerCutoff;
		float constant;
		float linear;
		float quadratic;
		bool isSpot;
	};

	std::vector<LightSetup> allLights;

	// Configure dual lighting system for enhanced visual impact
	for (int i = 0; i < 2; ++i) // NEW: Two lights instead of one
	{
		LightSetup light;
		light.index = i;
		light.constant = 1.0f;
		light.linear = 0.0f;
		light.quadratic = 0.0f;
		light.isSpot = false;

		if (i == 0)
		{
			// Primary dramatic spotlight configuration
			light.position = SceneConstants::BALANCED_LIGHT_POSITION;
			light.ambientColor = glm::vec3(SceneConstants::BALANCED_AMBIENT_INTENSITY);
			light.diffuseColor = glm::vec3(SceneConstants::BALANCED_DIFFUSE_INTENSITY);
			light.specularColor = glm::vec3(SceneConstants::BALANCED_SPECULAR_INTENSITY);
			light.focalStrength = 64.0f;
			light.specularIntensity = 1.0f;
			light.spotDirection = glm::normalize(SceneConstants::BALANCED_LIGHT_DIRECTION);
			light.cutoff = glm::cos(glm::radians(SceneConstants::BALANCED_SPOTLIGHT_INNER));
			light.outerCutoff = glm::cos(glm::radians(SceneConstants::BALANCED_SPOTLIGHT_OUTER));
			light.isSpot = true;
		}
		else if (i == 1)
		{
			// NEW: Secondary colored point light for accent lighting
			light.position = SceneConstants::ACCENT_LIGHT_POSITION;
			light.ambientColor = SceneConstants::ACCENT_LIGHT_AMBIENT;
			light.diffuseColor = SceneConstants::ACCENT_LIGHT_DIFFUSE;
			light.specularColor = SceneConstants::ACCENT_LIGHT_SPECULAR;
			light.focalStrength = 32.0f;
			light.specularIntensity = 0.8f;
			light.isSpot = false; // Point light, not spotlight
		}

		allLights.push_back(light);
	}

	// Apply dual lighting configuration to shader uniforms
	int numLights = 2; // NEW: Two lights for rubric compliance
	for (int i = 0; i < numLights; ++i)
	{
		const auto& light = allLights[i];
		std::string base = "lightSources[" + std::to_string(i) + "]";

		// Set all light properties using calculated values
		m_pShaderManager->setVec3Value(base + ".position", light.position);
		m_pShaderManager->setVec3Value(base + ".ambientColor", light.ambientColor);
		m_pShaderManager->setVec3Value(base + ".diffuseColor", light.diffuseColor);
		m_pShaderManager->setVec3Value(base + ".specularColor", light.specularColor);
		m_pShaderManager->setFloatValue(base + ".focalStrength", light.focalStrength);
		m_pShaderManager->setFloatValue(base + ".specularIntensity", light.specularIntensity);

		if (light.isSpot) {
			m_pShaderManager->setVec3Value(base + ".spotDirection", light.spotDirection);
			m_pShaderManager->setFloatValue(base + ".cutoff", light.cutoff);
			m_pShaderManager->setFloatValue(base + ".outerCutoff", light.outerCutoff);
		}
		m_pShaderManager->setFloatValue(base + ".constant", light.constant);
		m_pShaderManager->setFloatValue(base + ".linear", light.linear);
		m_pShaderManager->setFloatValue(base + ".quadratic", light.quadratic);
		
		// Debug output for light configuration verification
		std::cout << "Setting light uniform: " << base << ".position" << std::endl;
		std::cout << "Light " << i << " position: " << light.position.x << ", " << light.position.y << ", " << light.position.z << std::endl;
		if (i == 1) {
			std::cout << "Light " << i << " is COLORED BLUE accent light" << std::endl;
		}
	}
	
	// Configure shader with number of active lights
	m_pShaderManager->setIntValue("numActiveLights", numLights);

	// Debug output for active light count verification
	std::cout << "Setting numActiveLights to: " << numLights << " (Primary + Colored Accent)" << std::endl;

	// Initialize shadow mapping framebuffer and depth texture
	glGenFramebuffers(1, &m_shadowMapFBO);

	glGenTextures(1, &m_shadowMap);
	glBindTexture(GL_TEXTURE_2D, m_shadowMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1024, 1024, 0,
		GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	// Configure border color for areas outside shadow map
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// Attach depth texture to framebuffer for shadow map rendering
	glBindFramebuffer(GL_FRAMEBUFFER, m_shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_shadowMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Verify framebuffer completeness for shadow mapping
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "ERROR: Shadow map framebuffer is not complete!" << std::endl;
	}
	
	// Initialize projection matrix (add this at the end)
	UpdateProjectionMatrix();
	
	std::cout << "Scene prepared with projection support" << std::endl;
}

/***********************************************************
 *  InitShadowMap()
 *
 *  Creates and configures the framebuffer object and depth
 *  texture required for shadow mapping. Sets up texture
 *  parameters for proper shadow sampling and border handling.
 ***********************************************************/
void SceneManager::InitShadowMap()
{
	// Generate framebuffer for shadow map rendering
	glGenFramebuffers(1, &m_shadowMapFBO);

	// Create depth texture for shadow information storage
	glGenTextures(1, &m_shadowMap);
	glBindTexture(GL_TEXTURE_2D, m_shadowMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1024, 1024, 0,
		GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	// Set border color for areas outside shadow map coverage
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// Configure framebuffer for depth-only rendering
	glBindFramebuffer(GL_FRAMEBUFFER, m_shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_shadowMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/***********************************************************
 *  RenderDepthPass()
 *
 *  Performs depth-only rendering from the light's perspective
 *  to generate shadow map. Renders all shadow-casting objects
 *  including the new desk lamp using the depth shader with 
 *  front-face culling to reduce shadow acne artifacts.
 ***********************************************************/
void SceneManager::RenderDepthPass(glm::mat4 lightSpaceMatrix)
{
	// Configure viewport and framebuffer for shadow map generation
	glViewport(0, 0, 1024, 1024); // Match shadow map texture resolution
	glBindFramebuffer(GL_FRAMEBUFFER, m_shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	
	// Enable front-face culling to reduce shadow acne
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	// Use depth shader and configure light space transformation
	m_pDepthShaderManager->use();
	m_pDepthShaderManager->setMat4Value("lightSpaceMatrix", lightSpaceMatrix);

	// Render all shadow-casting objects using constants for consistency

	// Desk surface - main horizontal surface
	SetModelMatrix(SceneConstants::DESK_SCALE, 
		SceneConstants::DESK_ROTATION.x, SceneConstants::DESK_ROTATION.y, SceneConstants::DESK_ROTATION.z, 
		SceneConstants::DESK_POSITION);
	m_basicMeshes->DrawBoxMesh();

	// Coffee mug body (cylinder) - primary shadow caster
	SetModelMatrix(SceneConstants::MUG_BODY_SCALE, 
		SceneConstants::MUG_BODY_ROTATION.x, SceneConstants::MUG_BODY_ROTATION.y, SceneConstants::MUG_BODY_ROTATION.z, 
		SceneConstants::MUG_BODY_POSITION);
	m_basicMeshes->DrawCylinderMesh();

	// Coffee mug handle (torus) - secondary shadow caster
	SetModelMatrix(SceneConstants::MUG_HANDLE_SCALE, 
		SceneConstants::MUG_HANDLE_ROTATION.x, SceneConstants::MUG_HANDLE_ROTATION.y, SceneConstants::MUG_HANDLE_ROTATION.z, 
		SceneConstants::MUG_HANDLE_POSITION);
	m_basicMeshes->DrawTorusMesh();

	// NEW: Desk lamp components for shadow casting
	// Lamp base (cylinder)
	SetModelMatrix(SceneConstants::LAMP_BASE_SCALE,
		SceneConstants::LAMP_BASE_ROTATION.x, SceneConstants::LAMP_BASE_ROTATION.y, SceneConstants::LAMP_BASE_ROTATION.z,
		SceneConstants::LAMP_BASE_POSITION);
	m_basicMeshes->DrawCylinderMesh();

	// Lamp stem (cylinder)
	SetModelMatrix(SceneConstants::LAMP_STEM_SCALE,
		SceneConstants::LAMP_STEM_ROTATION.x, SceneConstants::LAMP_STEM_ROTATION.y, SceneConstants::LAMP_STEM_ROTATION.z,
		SceneConstants::LAMP_STEM_POSITION);
	m_basicMeshes->DrawCylinderMesh();

	// Lamp shade (cone)
	SetModelMatrix(SceneConstants::LAMP_SHADE_SCALE,
		SceneConstants::LAMP_SHADE_ROTATION.x, SceneConstants::LAMP_SHADE_ROTATION.y, SceneConstants::LAMP_SHADE_ROTATION.z,
		SceneConstants::LAMP_SHADE_POSITION);
	m_basicMeshes->DrawConeMesh();

	// Laptop base - flat rectangular component
	SetModelMatrix(SceneConstants::LAPTOP_BASE_SCALE, 
		SceneConstants::LAPTOP_BASE_ROTATION.x, SceneConstants::LAPTOP_BASE_ROTATION.y, SceneConstants::LAPTOP_BASE_ROTATION.z, 
		SceneConstants::LAPTOP_BASE_POSITION);
	m_basicMeshes->DrawBoxMesh();

	// Laptop screen - angled display component
	SetModelMatrix(SceneConstants::LAPTOP_SCREEN_SCALE, 
		SceneConstants::LAPTOP_SCREEN_ROTATION.x, SceneConstants::LAPTOP_SCREEN_ROTATION.y, SceneConstants::LAPTOP_SCREEN_ROTATION.z, 
		SceneConstants::LAPTOP_SCREEN_POSITION);
	m_basicMeshes->DrawBoxMesh();

	// Display panel - screen surface
	SetModelMatrix(SceneConstants::DISPLAY_PANEL_SCALE, 
		SceneConstants::DISPLAY_PANEL_ROTATION.x, SceneConstants::DISPLAY_PANEL_ROTATION.y, SceneConstants::DISPLAY_PANEL_ROTATION.z, 
		SceneConstants::DISPLAY_PANEL_POSITION);
	m_basicMeshes->DrawBoxMesh();

	// Individual keyboard keys using constants
	for (int r = 0; r < SceneConstants::KEY_ROWS; ++r)
	{
		for (int c = 0; c < SceneConstants::KEY_COLS; ++c)
		{
			float x = SceneConstants::KEY_START_X + c * (SceneConstants::KEY_WIDTH + SceneConstants::KEY_SPACING);
			float y = SceneConstants::KEY_Y;
			float z = SceneConstants::KEY_START_Z + r * (SceneConstants::KEY_DEPTH + SceneConstants::KEY_SPACING);

			SetModelMatrix(glm::vec3(SceneConstants::KEY_WIDTH, SceneConstants::KEY_HEIGHT, SceneConstants::KEY_DEPTH), 
				0.0f, 0.0f, 0.0f, glm::vec3(x, y, z));
			m_basicMeshes->DrawBoxMesh();
		}
	}

	// Floor - receives shadows from all objects
	SetModelMatrix(SceneConstants::FLOOR_SCALE, 
		SceneConstants::FLOOR_ROTATION.x, SceneConstants::FLOOR_ROTATION.y, SceneConstants::FLOOR_ROTATION.z, 
		SceneConstants::FLOOR_POSITION);
	m_basicMeshes->DrawBoxMesh();

	// Background wall - vertical surface for shadow projection
	float wallY = SceneConstants::FLOOR_Y + (SceneConstants::FLOOR_HEIGHT / 2.0f) + (SceneConstants::WALL_SCALE.y / 2.0f);
	glm::vec3 wallPosition = glm::vec3(SceneConstants::WALL_POSITION_OFFSET.x, wallY, SceneConstants::WALL_POSITION_OFFSET.z);
	SetModelMatrix(SceneConstants::WALL_SCALE, 
		SceneConstants::WALL_ROTATION.x, SceneConstants::WALL_ROTATION.y, SceneConstants::WALL_ROTATION.z, 
		wallPosition);
	m_basicMeshes->DrawBoxMesh();

	// Restore default culling settings and framebuffer
	glCullFace(GL_BACK);
	glDisable(GL_CULL_FACE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // Return to default framebuffer
}

/***********************************************************
 *  RenderScene()
 *
 *  Main scene rendering function with dual-light setup and
 *  dramatic lighting override for enhanced coffee mug shadows.
 *  Applies lighting configuration, shadow mapping, and renders
 *  all scene objects including the new desk lamp.
 ***********************************************************/
void SceneManager::RenderScene(glm::mat4 lightSpaceMatrix)
{
    std::cout << SceneConstants::DEBUG_RENDER_START << std::endl;
    
    m_pShaderManager->use();
    
    // Apply dramatic lighting override using constants
    std::cout << SceneConstants::DEBUG_LIGHTING_SETUP << std::endl;
    
    // Override lighting configuration for optimal coffee mug shadow effect with dual lights
    m_pShaderManager->setIntValue("numActiveLights", 2); // NEW: Two lights
    
    // Primary dramatic spotlight
    m_pShaderManager->setVec3Value("lightSources[0].position", SceneConstants::DRAMATIC_LIGHT_POSITION);
    m_pShaderManager->setVec3Value("lightSources[0].diffuseColor", glm::vec3(SceneConstants::DRAMATIC_LIGHT_INTENSITY));
    m_pShaderManager->setVec3Value("lightSources[0].spotDirection", SceneConstants::DRAMATIC_LIGHT_DIRECTION);
    m_pShaderManager->setFloatValue("lightSources[0].cutoff", glm::cos(glm::radians(SceneConstants::TIGHT_SPOTLIGHT_INNER)));
    m_pShaderManager->setFloatValue("lightSources[0].outerCutoff", glm::cos(glm::radians(SceneConstants::TIGHT_SPOTLIGHT_OUTER)));
    
    // NEW: Secondary colored accent light
    m_pShaderManager->setVec3Value("lightSources[1].position", SceneConstants::ACCENT_LIGHT_POSITION);
    m_pShaderManager->setVec3Value("lightSources[1].ambientColor", SceneConstants::ACCENT_LIGHT_AMBIENT);
    m_pShaderManager->setVec3Value("lightSources[1].diffuseColor", SceneConstants::ACCENT_LIGHT_DIFFUSE);
    m_pShaderManager->setVec3Value("lightSources[1].specularColor", SceneConstants::ACCENT_LIGHT_SPECULAR);
    
    std::cout << SceneConstants::DEBUG_LIGHTS_SET << std::endl;
    std::cout << SceneConstants::DEBUG_LIGHT_POSITION << std::endl;

    // Configure shadow mapping transformation matrix
    m_pShaderManager->setMat4Value("lightSpaceMatrix", lightSpaceMatrix);

    // Bind shadow map texture for shadow calculations in fragment shader
    const int SHADOW_MAP_SLOT = 15;
    glActiveTexture(GL_TEXTURE0 + SHADOW_MAP_SLOT);
    glBindTexture(GL_TEXTURE_2D, m_shadowMap);
    m_pShaderManager->setSampler2DValue("shadowMap", SHADOW_MAP_SLOT);

    std::cout << SceneConstants::DEBUG_DRAWING_OBJECTS << std::endl;
    
    // Render all scene objects with materials, textures, and lighting
    DrawDeskSurface();
    DrawCoffeeMug();
    DrawLaptop();
    DrawDeskLamp();        // NEW: Add the desk lamp rendering
    DrawFloor();
    DrawBackground();
    
    std::cout << SceneConstants::DEBUG_RENDER_END << std::endl;
}

/***********************************************************
 *  DrawDeskSurface()
 *
 *  Renders the wooden desk surface using constants for all
 *  transformations and textures. Creates the main horizontal 
 *  surface that receives shadows and provides workspace environment.
 ***********************************************************/
void SceneManager::DrawDeskSurface()
{
    std::cout << "Drawing desk..." << std::endl;
    
    // Configure desk using constants for consistency
    SetShaderTexture(SceneConstants::DESK_TEXTURE);
    SetTextureUVScale(SceneConstants::DEFAULT_UV_SCALE.x, SceneConstants::DEFAULT_UV_SCALE.y);
    SetShaderMaterial(SceneConstants::DESK_TEXTURE);
    m_pShaderManager->setIntValue("bUseLighting", true);

    // Position and scale desk using constants
    SetTransformations(
        SceneConstants::DESK_SCALE,
        SceneConstants::DESK_ROTATION.x, SceneConstants::DESK_ROTATION.y, SceneConstants::DESK_ROTATION.z,
        SceneConstants::DESK_POSITION
    );

    m_basicMeshes->DrawBoxMesh();
    std::cout << "Desk drawn successfully" << std::endl;
}

/***********************************************************
 *  DrawCoffeeMug()
 *
 *  Renders the complete coffee mug assembly using constants
 *  for all transformations, textures, and materials. Creates
 *  optimal shadow effects from the dramatic lighting.
 ***********************************************************/
void SceneManager::DrawCoffeeMug()
{
    std::cout << "Drawing mug..." << std::endl;
    
    // Coffee mug body (cylinder) using constants
    SetShaderTexture(SceneConstants::MUG_TEXTURE);
    SetTextureUVScale(SceneConstants::DEFAULT_UV_SCALE.x, SceneConstants::DEFAULT_UV_SCALE.y);
    SetShaderMaterial(SceneConstants::MUG_TEXTURE);
    m_pShaderManager->setIntValue("bUseLighting", true);

    // Position and rotate mug using constants
    SetTransformations(
        SceneConstants::MUG_BODY_SCALE,
        SceneConstants::MUG_BODY_ROTATION.x, SceneConstants::MUG_BODY_ROTATION.y, SceneConstants::MUG_BODY_ROTATION.z,
        SceneConstants::MUG_BODY_POSITION
    );
    m_basicMeshes->DrawCylinderMesh();

    // Coffee mug handle (torus) using constants
    SetShaderTexture(SceneConstants::HANDLE_TEXTURE);
    SetTextureUVScale(SceneConstants::DEFAULT_UV_SCALE.x, SceneConstants::DEFAULT_UV_SCALE.y);
    SetShaderMaterial(SceneConstants::MUG_TEXTURE); // Reuse mug material

    // Position handle using constants
    SetTransformations(
        SceneConstants::MUG_HANDLE_SCALE,
        SceneConstants::MUG_HANDLE_ROTATION.x, SceneConstants::MUG_HANDLE_ROTATION.y, SceneConstants::MUG_HANDLE_ROTATION.z,
        SceneConstants::MUG_HANDLE_POSITION
    );
    m_basicMeshes->DrawTorusMesh();
    
    std::cout << "Complete mug drawn successfully" << std::endl;
}

/***********************************************************
 *  DrawLaptop()
 *
 *  Renders detailed laptop assembly using constants for all
 *  transformations. Includes base, angled screen, display panel,
 *  and 60 individual keyboard keys for realistic appearance.
 ***********************************************************/
void SceneManager::DrawLaptop()
{
    std::cout << "Drawing laptop..." << std::endl;
    
    // Laptop base using constants
    SetShaderColor(SceneConstants::METAL_AMBIENT_COLOR.x, SceneConstants::METAL_AMBIENT_COLOR.y, SceneConstants::METAL_AMBIENT_COLOR.z, 1.0f);
    SetShaderTexture(SceneConstants::LAPTOP_TEXTURE);
    SetTextureUVScale(SceneConstants::LAPTOP_UV_SCALE.x, SceneConstants::LAPTOP_UV_SCALE.y);
    SetShaderMaterial(SceneConstants::LAPTOP_TEXTURE);
    m_pShaderManager->setIntValue("bUseLighting", true);

    // Position laptop base using constants
    SetTransformations(
        SceneConstants::LAPTOP_BASE_SCALE,
        SceneConstants::LAPTOP_BASE_ROTATION.x, SceneConstants::LAPTOP_BASE_ROTATION.y, SceneConstants::LAPTOP_BASE_ROTATION.z,
        SceneConstants::LAPTOP_BASE_POSITION
    );
    m_basicMeshes->DrawBoxMesh();

    // Laptop screen assembly using constants
    SetShaderTexture(SceneConstants::LAPTOP_TEXTURE);
    SetShaderMaterial(SceneConstants::LAPTOP_TEXTURE);

    // Position screen using constants
    SetTransformations(
        SceneConstants::LAPTOP_SCREEN_SCALE,
        SceneConstants::LAPTOP_SCREEN_ROTATION.x, SceneConstants::LAPTOP_SCREEN_ROTATION.y, SceneConstants::LAPTOP_SCREEN_ROTATION.z,
        SceneConstants::LAPTOP_SCREEN_POSITION
    );
    m_basicMeshes->DrawBoxMesh();

    // Display panel - screen surface
    SetShaderTexture(SceneConstants::SCREEN_TEXTURE);
    SetTextureUVScale(SceneConstants::DEFAULT_UV_SCALE.x, SceneConstants::DEFAULT_UV_SCALE.y);
    SetShaderMaterial(SceneConstants::LAPTOP_TEXTURE);

    // Position display panel using constants
    SetTransformations(
        SceneConstants::DISPLAY_PANEL_SCALE,
        SceneConstants::DISPLAY_PANEL_ROTATION.x, SceneConstants::DISPLAY_PANEL_ROTATION.y, SceneConstants::DISPLAY_PANEL_ROTATION.z,
        SceneConstants::DISPLAY_PANEL_POSITION
    );
    m_basicMeshes->DrawBoxMesh();

    // Individual keyboard keys - 60 keys for maximum detail and lighting using constants
    for (int r = 0; r < SceneConstants::KEY_ROWS; ++r)
    {
        for (int c = 0; c < SceneConstants::KEY_COLS; ++c)
        {
            // Calculate позиция for each key in grid pattern using constants
            float x = SceneConstants::KEY_START_X + c * (SceneConstants::KEY_WIDTH + SceneConstants::KEY_SPACING);
            float y = SceneConstants::KEY_Y;
            float z = SceneConstants::KEY_START_Z + r * (SceneConstants::KEY_DEPTH + SceneConstants::KEY_SPACING);

            SetTransformations(
                glm::vec3(SceneConstants::KEY_WIDTH, SceneConstants::KEY_HEIGHT, SceneConstants::KEY_DEPTH),
                0.0f, 0.0f, 0.0f,
                glm::vec3(x, y, z)
            );

            // Keys use solid dark color for contrast with textured surfaces
            SetShaderColor(SceneConstants::KEY_COLOR.r, SceneConstants::KEY_COLOR.g, SceneConstants::KEY_COLOR.b, 1.0f);
            m_pShaderManager->setIntValue("bUseLighting", true); // Enable lighting for individual shadows
            m_basicMeshes->DrawBoxMesh();
        }
    }
    
    std::cout << "Complete laptop drawn successfully" << std::endl;
}

/***********************************************************
 *  DrawFloor()
 *
 *  Renders the floor surface with paver stone texture and
 *  tiling. Creates the base surface that receives shadows
 *  from all objects and displays the dramatic lighting
 *  patterns throughout the scene.
 ***********************************************************/
void SceneManager::DrawFloor()
{
	// Configure floor with tiled stone texture using constants
	SetShaderTexture(SceneConstants::FLOOR_TEXTURE);
	SetTextureUVScale(SceneConstants::FLOOR_UV_SCALE.x, SceneConstants::FLOOR_UV_SCALE.y);
	m_pShaderManager->setIntValue("bUseLighting", true); // Enable lighting for shadow reception

	// Position floor as large base surface using constants
	SetTransformations(
		SceneConstants::FLOOR_SCALE,
		SceneConstants::FLOOR_ROTATION.x, SceneConstants::FLOOR_ROTATION.y, SceneConstants::FLOOR_ROTATION.z,
		SceneConstants::FLOOR_POSITION
	);

	SetShaderMaterial(SceneConstants::FLOOR_TEXTURE); // Stone material properties
	m_basicMeshes->DrawBoxMesh();
}

/***********************************************************
 *  DrawBackground()
 *
 *  Renders the background wall with backdrop texture and
 *  dramatic lighting effects. Provides visual context and
 *  depth to the scene while receiving lighting to help
 *  define the scene boundaries and lighting coverage.
 ***********************************************************/
void SceneManager::DrawBackground()
{
	// Configure wall with backdrop texture using constants
	SetShaderTexture(SceneConstants::WALL_TEXTURE);
	SetTextureUVScale(SceneConstants::WALL_UV_SCALE.x, SceneConstants::WALL_UV_SCALE.y);
	SetShaderMaterial(SceneConstants::WALL_TEXTURE);
	m_pShaderManager->setIntValue("bUseLighting", true); // Enable lighting for wall illumination

	// Calculate wall position relative to floor level using constants
	float wallWidth = 62.4f;
	float wallHeight = 26.0f;
	float wallThickness = 0.65f;
	float wallDepth = -21.25f;

	float floorY = SceneConstants::FLOOR_Y;
	float floorHeight = SceneConstants::FLOOR_HEIGHT;
	float wallY = floorY + (floorHeight / 2.0f) + (wallHeight / 2.0f);

	// Position wall as vertical backdrop using constants
	SetTransformations(
		glm::vec3(wallWidth, wallHeight, wallThickness),
		SceneConstants::WALL_ROTATION.x, SceneConstants::WALL_ROTATION.y, SceneConstants::WALL_ROTATION.z,
		glm::vec3(0.0f, wallY, wallDepth)
	);	

	m_basicMeshes->DrawBoxMesh();
}

/***********************************************************
 *  DrawDeskLamp()
 *
 *  Renders a complete desk lamp assembly using cone and cylinder
 *  primitives. Uses golden brushed texture for the shade and
 *  stainless steel texture for the base and stem components.
 *  Adds the required additional primitive shape (cone) to 
 *  satisfy rubric requirements.
 ***********************************************************/
void SceneManager::DrawDeskLamp()
{
    std::cout << "Drawing desk lamp..." << std::endl;
    
    // Lamp base (cylinder) using stainless steel texture
    SetShaderTexture(SceneConstants::LAMP_BASE_TEXTURE);
    SetTextureUVScale(SceneConstants::DEFAULT_UV_SCALE.x, SceneConstants::DEFAULT_UV_SCALE.y);
    SetShaderMaterial(SceneConstants::LAMP_BASE_TEXTURE);
    m_pShaderManager->setIntValue("bUseLighting", true);

    // Position lamp base using constants
    SetTransformations(
        SceneConstants::LAMP_BASE_SCALE,
        SceneConstants::LAMP_BASE_ROTATION.x, SceneConstants::LAMP_BASE_ROTATION.y, SceneConstants::LAMP_BASE_ROTATION.z,
        SceneConstants::LAMP_BASE_POSITION
    );
    m_basicMeshes->DrawCylinderMesh();

    // Lamp stem (cylinder) using same stainless steel texture
    SetTransformations(
        SceneConstants::LAMP_STEM_SCALE,
        SceneConstants::LAMP_STEM_ROTATION.x, SceneConstants::LAMP_STEM_ROTATION.y, SceneConstants::LAMP_STEM_ROTATION.z,
        SceneConstants::LAMP_STEM_POSITION
    );
    m_basicMeshes->DrawCylinderMesh();

    // Lamp shade (cone) using golden brushed texture - NEW PRIMITIVE SHAPE
    SetShaderTexture(SceneConstants::LAMP_SHADE_TEXTURE);
    SetTextureUVScale(SceneConstants::DEFAULT_UV_SCALE.x, SceneConstants::DEFAULT_UV_SCALE.y);
    SetShaderMaterial(SceneConstants::LAMP_SHADE_TEXTURE);

    // Position lamp shade using constants
    SetTransformations(
        SceneConstants::LAMP_SHADE_SCALE,
        SceneConstants::LAMP_SHADE_ROTATION.x, SceneConstants::LAMP_SHADE_ROTATION.y, SceneConstants::LAMP_SHADE_ROTATION.z,
        SceneConstants::LAMP_SHADE_POSITION
    );
    m_basicMeshes->DrawConeMesh(); // NEW: Cone primitive for rubric requirement
    
    std::cout << "Desk lamp drawn successfully" << std::endl;
}

/***********************************************************
 *  SetProjectionMode()
 *
 *  Toggles between perspective and orthographic projection
 *  modes and updates the projection matrix accordingly.
 *********************************************************/
void SceneManager::SetProjectionMode(bool isPerspective)
{
    m_isPerspectiveProjection = isPerspective;
    UpdateProjectionMatrix();
    
    // Apply projection matrix to shader
    if (m_pShaderManager)
    {
        m_pShaderManager->SetProjectionMatrix(m_projectionMatrix);
    }
    
    std::cout << "Projection mode: " << (isPerspective ? "Perspective" : "Orthographic") << std::endl;
}

/***********************************************************
 *  SetScreenDimensions()
 *
 *  Updates screen dimensions for projection calculations.
 ***********************************************************/
void SceneManager::SetScreenDimensions(float width, float height)
{
    m_screenWidth = width;
    m_screenHeight = height;
    UpdateProjectionMatrix();
}

/***********************************************************
 *  UpdateProjectionMatrix()
 *
 *  Calculates the appropriate projection matrix based on
 *  current projection mode and screen dimensions.
 ***********************************************************/
void SceneManager::UpdateProjectionMatrix()
{
    float aspectRatio = m_screenWidth / m_screenHeight;
    
    if (m_isPerspectiveProjection)
    {
        // Perspective projection
        float fov = 45.0f; // Field of view in degrees
        float nearPlane = 0.1f;
        float farPlane = 100.0f;
        
        m_projectionMatrix = glm::perspective(
            glm::radians(fov), 
            aspectRatio, 
            nearPlane, 
            farPlane
        );
    }
    else
    {
        // Orthographic projection
        // Adjust these values based on your scene scale
        float orthoSize = 20.0f; // Controls zoom level
        float left = -orthoSize * aspectRatio;
        float right = orthoSize * aspectRatio;
        float bottom = -orthoSize;
        float top = orthoSize;
        float nearPlane = 0.1f;
        float farPlane = 100.0f;
        
        m_projectionMatrix = glm::ortho(
            left, right, 
            bottom, top, 
            nearPlane, farPlane
        );
    }
    
    // Update shader with new projection matrix
    if (m_pShaderManager)
    {
        m_pShaderManager->SetProjectionMatrix(m_projectionMatrix);
    }
}
