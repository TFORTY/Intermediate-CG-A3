//Just a simple handler for simple initialization stuffs
#include "Utilities/BackendHandler.h"

#include <filesystem>
#include <json.hpp>
#include <fstream>

//TODO: New for this tutorial
#include <DirectionalLight.h>
#include <PointLight.h>
#include <UniformBuffer.h>
/////////////////////////////

#include <Texture2D.h>
#include <Texture2DData.h>
#include <MeshBuilder.h>
#include <MeshFactory.h>
#include <NotObjLoader.h>
#include <ObjLoader.h>
#include <VertexTypes.h>
#include <ShaderMaterial.h>
#include <RendererComponent.h>
#include <TextureCubeMap.h>
#include <TextureCubeMapData.h>

#include <Timing.h>
#include <GameObjectTag.h>
#include <InputHelpers.h>

#include <IBehaviour.h>
#include <CameraControlBehaviour.h>
#include <FollowPathBehaviour.h>
#include <SimpleMoveBehaviour.h>

int main() {
	int frameIx = 0;
	float fpsBuffer[128];
	float minFps, maxFps, avgFps;
	int selectedVao = 0; // select cube by default
	std::vector<GameObject> controllables;

	bool drawGBuffer = false;
	bool drawAlbedoBuffer = false;  
	bool drawNormalsBuffer = false; 
	bool drawPositionsBuffer = false;
	bool drawSpecularBuffer = false;
	bool drawIllumBuffer = false; 

	BackendHandler::InitAll();
	 
	// Let OpenGL know that we want debug output, and route it to our handler function
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(BackendHandler::GlDebugMessage, nullptr);

	// Enable texturing
	glEnable(GL_TEXTURE_2D); 

	// Push another scope so most memory should be freed *before* we exit the app
	{
		#pragma region Shader and ImGui
		Shader::sptr passthroughShader = Shader::Create();
		passthroughShader->LoadShaderPartFromFile("shaders/passthrough_vert.glsl", GL_VERTEX_SHADER);
		passthroughShader->LoadShaderPartFromFile("shaders/passthrough_frag.glsl", GL_FRAGMENT_SHADER);
		passthroughShader->Link();

		Shader::sptr simpleDepthShader = Shader::Create();
		simpleDepthShader->LoadShaderPartFromFile("shaders/simple_depth_vert.glsl", GL_VERTEX_SHADER);
		simpleDepthShader->LoadShaderPartFromFile("shaders/simple_depth_frag.glsl", GL_FRAGMENT_SHADER);
		simpleDepthShader->Link();

		//Init gBuffer shader
		Shader::sptr gBufferShader = Shader::Create();
		gBufferShader->LoadShaderPartFromFile("shaders/vertex_shader.glsl", GL_VERTEX_SHADER);
		gBufferShader->LoadShaderPartFromFile("shaders/gBuffer_pass_frag.glsl", GL_FRAGMENT_SHADER);
		gBufferShader->Link();

		// Load our shaders
		Shader::sptr shader = Shader::Create();
		shader->LoadShaderPartFromFile("shaders/vertex_shader.glsl", GL_VERTEX_SHADER);
		//Directional Light Shader
		shader->LoadShaderPartFromFile("shaders/directional_blinn_phong_frag.glsl", GL_FRAGMENT_SHADER);
		shader->Link();

		////Creates our directional Light
		//DirectionalLight theSun;
		//UniformBuffer directionalLightBuffer;

		////Allocates enough memory for one directional light (we can change this easily, but we only need 1 directional light)
		//directionalLightBuffer.AllocateMemory(sizeof(DirectionalLight));
		////Casts our sun as "data" and sends it to the shader
		//directionalLightBuffer.SendData(reinterpret_cast<void*>(&theSun), sizeof(DirectionalLight));

		//directionalLightBuffer.Bind(0);

		//Basic effect for drawing to
		PostEffect* basicEffect;
		Framebuffer* shadowBuffer;
		GBuffer* gBuffer;
		IlluminationBuffer* illuminationBuffer;

		//Post Processing Effects
		int activeEffect = 0;
		std::vector<PostEffect*> effects;
		SepiaEffect* sepiaEffect;
		GreyscaleEffect* greyscaleEffect;
		ColorCorrectEffect* colorCorrectEffect;
		BloomEffect* bloomEffect;
		FilmGrainEffect* filmGrainEffect;
		PixelateEffect* pixelateEffect;
		
		// We'll add some ImGui controls to control our shader
		BackendHandler::imGuiCallbacks.push_back([&]() {
			if (ImGui::CollapsingHeader("Effect controls"))
			{
				ImGui::SliderInt("Chosen Effect", &activeEffect, 0, effects.size() - 1);

				if (activeEffect == 0)
				{
					ImGui::Text("Active Effect: No Effect");

					PostEffect* temp = (PostEffect*)effects[activeEffect];
				}
				if (activeEffect == 1)
				{
					ImGui::Text("Active Effect: Sepia Effect");

					SepiaEffect* temp = (SepiaEffect*)effects[activeEffect];
					float intensity = temp->GetIntensity();

					if (ImGui::SliderFloat("Intensity", &intensity, 0.0f, 1.0f))
					{
						temp->SetIntensity(intensity);
					}
				}
				if (activeEffect == 2)
				{
					ImGui::Text("Active Effect: Greyscale Effect");
					
					GreyscaleEffect* temp = (GreyscaleEffect*)effects[activeEffect];
					float intensity = temp->GetIntensity();

					if (ImGui::SliderFloat("Intensity", &intensity, 0.0f, 1.0f))
					{
						temp->SetIntensity(intensity);
					}
				}
				if (activeEffect == 3)
				{
					ImGui::Text("Active Effect: Color Correct Effect");

					ColorCorrectEffect* temp = (ColorCorrectEffect*)effects[activeEffect];
				}
				if (activeEffect == 4)
				{
					ImGui::Text("Active Effect: Bloom Effect");

					BloomEffect* temp = (BloomEffect*)effects[activeEffect];
					float brightnessThreshold = temp->GetThreshold();
					int blurValue = temp->GetPasses();

					if (ImGui::SliderFloat("Brightness Threshold", &brightnessThreshold, 0.0f, 1.0f))
					{
						temp->SetThreshold(brightnessThreshold);
					}
					if (ImGui::SliderInt("Blur Value", &blurValue, 0, 20))
					{
						temp->SetPasses(blurValue);
					}
				}
				if (activeEffect == 5)
				{
					ImGui::Text("Active Effect: Film Grain Effect");
					
					FilmGrainEffect* temp = (FilmGrainEffect*)effects[activeEffect];
					float strength = temp->GetStrength();

					if (ImGui::SliderFloat("Strength", &strength, 0.0f, 64.0f))
					{
						temp->SetStrength(strength);
					}
				}
				if (activeEffect == 6)
				{
					ImGui::Text("Active Effect: Pixelate Effect");

					PixelateEffect* temp = (PixelateEffect*)effects[activeEffect];
					float pixelSize = temp->GetPixelSize();

					if (ImGui::SliderFloat("Pixel Size", &pixelSize, 0.1f, 32.f))
					{
						temp->SetPixelSize(pixelSize);
					}				
				}
			}

			if (ImGui::CollapsingHeader("GBuffer Toggles"))
			{
				if (ImGui::Checkbox("Albedo Buffer", &drawAlbedoBuffer))
				{ 
					drawNormalsBuffer = false;
					drawPositionsBuffer = false;
					drawSpecularBuffer = false;
					drawGBuffer = false;
					drawIllumBuffer = false;
				}							
				if (ImGui::Checkbox("Normals Buffer", &drawNormalsBuffer))
				{ 
					drawAlbedoBuffer = false;
					drawPositionsBuffer = false;
					drawSpecularBuffer = false;
					drawGBuffer = false;
					drawIllumBuffer = false;
				}
				if (ImGui::Checkbox("Positions Buffer", &drawPositionsBuffer))
				{ 
					drawAlbedoBuffer = false;
					drawNormalsBuffer = false;
					drawSpecularBuffer = false;
					drawGBuffer = false;
					drawIllumBuffer = false;
				}
				if (ImGui::Checkbox("Specular Buffer", &drawSpecularBuffer))
				{
					drawAlbedoBuffer = false;
					drawNormalsBuffer = false;
					drawPositionsBuffer = false;
					drawGBuffer = false;
					drawIllumBuffer = false;
				}
				if (ImGui::Checkbox("All G-Buffers", &drawGBuffer))
				{
					drawAlbedoBuffer = false;
					drawNormalsBuffer = false;
					drawPositionsBuffer = false;
					drawSpecularBuffer = false;
					drawIllumBuffer = false;
				}
				if (ImGui::Checkbox("Light Accumulation Buffer", &drawIllumBuffer))
				{
					drawAlbedoBuffer = false;
					drawNormalsBuffer = false;
					drawPositionsBuffer = false;
					drawSpecularBuffer = false;
					drawGBuffer = false;
				}
			}

			if (ImGui::CollapsingHeader("Light Level Lighting Settings"))
			{
				if (ImGui::DragFloat3("Light Direction/Position", glm::value_ptr(illuminationBuffer->GetSunRef()._lightDirection), 0.01f, -100.0f, 100.0f))
				{
				}
				if (ImGui::DragFloat("Ambience", &(illuminationBuffer->GetSunRef()._ambientPow), 0.01f, 0.0, 1.0f))
				{
				}
			}

			auto name = controllables[selectedVao].get<GameObjectTag>().Name;
			ImGui::Text(name.c_str());
			auto behaviour = BehaviourBinding::Get<SimpleMoveBehaviour>(controllables[selectedVao]);
			ImGui::Checkbox("Relative Rotation", &behaviour->Relative);

			ImGui::Text("Q/E -> Yaw\nLeft/Right -> Roll\nUp/Down -> Pitch\nY -> Toggle Mode");
		
			minFps = FLT_MAX;
			maxFps = 0;
			avgFps = 0;
			for (int ix = 0; ix < 128; ix++) {
				if (fpsBuffer[ix] < minFps) { minFps = fpsBuffer[ix]; }
				if (fpsBuffer[ix] > maxFps) { maxFps = fpsBuffer[ix]; }
				avgFps += fpsBuffer[ix];
			}
			ImGui::PlotLines("FPS", fpsBuffer, 128);
			ImGui::Text("MIN: %f MAX: %f AVG: %f", minFps, maxFps, avgFps / 128.0f);
			});

		#pragma endregion 

		// GL states
		glEnable(GL_DEPTH_TEST);
		//glEnable(GL_CULL_FACE);
		glDepthFunc(GL_LEQUAL); // New 

		///////////////////////////////////// Texture Loading //////////////////////////////////////////////////
		#pragma region Texture

		// Load some textures from files
		
		Texture2D::sptr Baselevelobjects = Texture2D::LoadFromFile("images/FloorObjects_color.png");
		Texture2D::sptr Baselevel = Texture2D::LoadFromFile("images/BaseLevel_color.png");
		Texture2D::sptr Building2 = Texture2D::LoadFromFile("images/Building_color.png");		
		Texture2D::sptr Building = Texture2D::LoadFromFile("images/Building2_color.png");
		Texture2D::sptr Floor = Texture2D::LoadFromFile("images/Floor_color.png");
		Texture2D::sptr Monitor = Texture2D::LoadFromFile("images/Monitor-Albedo.png");
		Texture2D::sptr Tower = Texture2D::LoadFromFile("images/Tower_color.png");
		Texture2D::sptr Drumstick = Texture2D::LoadFromFile("images/DrumstickTexture.png");
		Texture2D::sptr ControlPanel = Texture2D::LoadFromFile("images/ControlPanel_color.jpg");
		Texture2D::sptr Stairs = Texture2D::LoadFromFile("images/Stairs_color.png");
		
		Texture2D::sptr ControlPanelSpec = Texture2D::LoadFromFile("images/ControlPanel_Specular.jpg");
		Texture2D::sptr noSpec = Texture2D::LoadFromFile("images/noSpec.png");
		

		LUT3D testCube("cubes/BrightenedCorrection.cube");

		// Load the cube map
		//TextureCubeMap::sptr environmentMap = TextureCubeMap::LoadFromImages("images/cubemaps/skybox/sample.jpg");
		TextureCubeMap::sptr environmentMap = TextureCubeMap::LoadFromImages("images/cubemaps/skybox/ToonSky.jpg"); 

		// Creating an empty texture
		Texture2DDescription desc = Texture2DDescription();  
		desc.Width = 1;
		desc.Height = 1;
		desc.Format = InternalFormat::RGB8;
		Texture2D::sptr texture2 = Texture2D::Create(desc);
		// Clear it with a white colour
		texture2->Clear();

		#pragma endregion
		//////////////////////////////////////////////////////////////////////////////////////////

		///////////////////////////////////// Scene Generation //////////////////////////////////////////////////
		#pragma region Scene Generation
		
		// We need to tell our scene system what extra component types we want to support
		GameScene::RegisterComponentType<RendererComponent>();
		GameScene::RegisterComponentType<BehaviourBinding>();
		GameScene::RegisterComponentType<Camera>();

		// Create a scene, and set it to be the active scene in the application
		GameScene::sptr scene = GameScene::Create("test");
		Application::Instance().ActiveScene = scene;

		// We can create a group ahead of time to make iterating on the group faster
		entt::basic_group<entt::entity, entt::exclude_t<>, entt::get_t<Transform>, RendererComponent> renderGroup =
			scene->Registry().group<RendererComponent>(entt::get_t<Transform>());

		// Create a material and set some properties for it
		ShaderMaterial::sptr baselevelObjectsMat = ShaderMaterial::Create();  
		ShaderMaterial::sptr baselevelMat = ShaderMaterial::Create();  
		ShaderMaterial::sptr buildingMat = ShaderMaterial::Create();  
		ShaderMaterial::sptr building2Mat = ShaderMaterial::Create();  
		ShaderMaterial::sptr floorMat = ShaderMaterial::Create();  
		ShaderMaterial::sptr monitorMat = ShaderMaterial::Create();  
		ShaderMaterial::sptr towerMat = ShaderMaterial::Create();  
		ShaderMaterial::sptr drumstickMat = ShaderMaterial::Create();  
		ShaderMaterial::sptr controlpanelMat = ShaderMaterial::Create();  
		ShaderMaterial::sptr stairsMat = ShaderMaterial::Create();  

		//stoneMat->Shader = shader;
		baselevelMat->Shader = gBufferShader;
		baselevelMat->Set("s_Diffuse", Baselevel);
		baselevelMat->Set("s_Specular", noSpec);
		baselevelMat->Set("u_Shininess", 2.0f);
		baselevelMat->Set("u_TextureMix", 0.0f); 
		
		baselevelObjectsMat->Shader = gBufferShader;
		baselevelObjectsMat->Set("s_Diffuse", Baselevelobjects);
		baselevelObjectsMat->Set("s_Specular", noSpec);
		baselevelObjectsMat->Set("u_Shininess", 2.0f);
		baselevelObjectsMat->Set("u_TextureMix", 0.0f); 
		
		buildingMat->Shader = gBufferShader;
		buildingMat->Set("s_Diffuse", Building);
		buildingMat->Set("s_Specular", noSpec);
		buildingMat->Set("u_Shininess", 2.0f);
		buildingMat->Set("u_TextureMix", 0.0f); 
		
		building2Mat->Shader = gBufferShader;
		building2Mat->Set("s_Diffuse", Building2);
		building2Mat->Set("s_Specular", noSpec);
		building2Mat->Set("u_Shininess", 2.0f);
		building2Mat->Set("u_TextureMix", 0.0f); 
		
		floorMat->Shader = gBufferShader;
		floorMat->Set("s_Diffuse", Floor);
		floorMat->Set("s_Specular", noSpec);
		floorMat->Set("u_Shininess", 2.0f);
		floorMat->Set("u_TextureMix", 0.0f); 
		
		monitorMat->Shader = gBufferShader;
		monitorMat->Set("s_Diffuse", Monitor);
		monitorMat->Set("s_Specular", noSpec);
		monitorMat->Set("u_Shininess", 4.0f);
		monitorMat->Set("u_TextureMix", 0.0f); 
		
		towerMat->Shader = gBufferShader;
		towerMat->Set("s_Diffuse", Tower);
		towerMat->Set("s_Specular", noSpec);
		towerMat->Set("u_Shininess", 2.0f);
		towerMat->Set("u_TextureMix", 0.0f); 
		
		drumstickMat->Shader = gBufferShader;
		drumstickMat->Set("s_Diffuse", Drumstick);
		drumstickMat->Set("s_Specular", noSpec);
		drumstickMat->Set("u_Shininess", 2.0f);
		drumstickMat->Set("u_TextureMix", 0.0f); 
		
		controlpanelMat->Shader = gBufferShader;
		controlpanelMat->Set("s_Diffuse", ControlPanel);
		controlpanelMat->Set("s_Specular", ControlPanelSpec);
		controlpanelMat->Set("u_Shininess", 4.0f);
		controlpanelMat->Set("u_TextureMix", 0.0f); 	
		
		stairsMat->Shader = gBufferShader;
		stairsMat->Set("s_Diffuse", Stairs);
		stairsMat->Set("s_Specular", noSpec);
		stairsMat->Set("u_Shininess", 4.0f);
		stairsMat->Set("u_TextureMix", 0.0f); 

		GameObject obj = scene->CreateEntity("Ground Floor");
		{
			VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/plane.obj");
			obj.emplace<RendererComponent>().SetMesh(vao).SetMaterial(baselevelMat);
			obj.get<Transform>().SetLocalPosition(0.0f, 0.0f, 0.0f);
			obj.get<Transform>().SetLocalRotation(90.0f, 0.0f, 0.0f);
		}
		GameObject obj1 = scene->CreateEntity("Floor"); 
		{
			VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/Floor.obj");
			obj1.emplace<RendererComponent>().SetMesh(vao).SetMaterial(floorMat);
			obj1.get<Transform>().SetLocalPosition(0.0f, 0.0f, 0.0f);
			obj1.get<Transform>().SetLocalRotation(90.0f, 0.0f, 0.0f);
		}
		GameObject obj2 = scene->CreateEntity("Building");
		{
			VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/Building.obj");
			obj2.emplace<RendererComponent>().SetMesh(vao).SetMaterial(buildingMat);
			obj2.get<Transform>().SetLocalPosition(0.0f, 0.0f, 0.0f);
			obj2.get<Transform>().SetLocalRotation(90.0f, 0.0f, 0.0f);
			BehaviourBinding::BindDisabled<SimpleMoveBehaviour>(obj2);
		}
		GameObject obj3 = scene->CreateEntity("Building2");
		{
			VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/Building2.obj");
			obj3.emplace<RendererComponent>().SetMesh(vao).SetMaterial(building2Mat);
			obj3.get<Transform>().SetLocalPosition(0.0f, 0.0f, 0.0f);
			obj3.get<Transform>().SetLocalRotation(90.0f, 0.0f, 0.0f);
			BehaviourBinding::BindDisabled<SimpleMoveBehaviour>(obj3);
		}
		GameObject obj4 = scene->CreateEntity("Computer");
		{
			VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/Computer.obj");
			obj4.emplace<RendererComponent>().SetMesh(vao).SetMaterial(monitorMat);
			obj4.get<Transform>().SetLocalPosition(0.0f, 0.0f, 0.0f);
			obj4.get<Transform>().SetLocalRotation(90.0f, 0.0f, 0.0f);
			BehaviourBinding::BindDisabled<SimpleMoveBehaviour>(obj4);
		}
		GameObject obj5 = scene->CreateEntity("Control Panel");
		{
			VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/Control Panel.obj");
			obj5.emplace<RendererComponent>().SetMesh(vao).SetMaterial(controlpanelMat);
			obj5.get<Transform>().SetLocalPosition(0.0f, 0.0f, 0.0f);
			obj5.get<Transform>().SetLocalRotation(90.0f, 0.0f, 0.0f);
			BehaviourBinding::BindDisabled<SimpleMoveBehaviour>(obj5);
		}
		GameObject obj6 = scene->CreateEntity("Drumstick");
		{
			VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/Drumstick.obj");
			obj6.emplace<RendererComponent>().SetMesh(vao).SetMaterial(drumstickMat);
			obj6.get<Transform>().SetLocalPosition(0.0f, 0.0f, 0.0f);
			obj6.get<Transform>().SetLocalRotation(90.0f, 0.0f, 0.0f);
			BehaviourBinding::BindDisabled<SimpleMoveBehaviour>(obj6);
		}
		GameObject obj7 = scene->CreateEntity("Stairs");
		{
			VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/Stairs.obj");
			obj7.emplace<RendererComponent>().SetMesh(vao).SetMaterial(stairsMat);
			obj7.get<Transform>().SetLocalPosition(0.0f, 0.0f, 0.0f);
			obj7.get<Transform>().SetLocalRotation(90.0f, 0.0f, 0.0f);
			BehaviourBinding::BindDisabled<SimpleMoveBehaviour>(obj7);
		}
		GameObject obj8 = scene->CreateEntity("Tower");
		{
			VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/Tower.obj");
			obj8.emplace<RendererComponent>().SetMesh(vao).SetMaterial(towerMat);
			obj8.get<Transform>().SetLocalPosition(0.0f, 0.0f, 0.0f);
			obj8.get<Transform>().SetLocalRotation(90.0f, 0.0f, 0.0f);
			BehaviourBinding::BindDisabled<SimpleMoveBehaviour>(obj8);
		}
		GameObject obj9 = scene->CreateEntity("BaseFloorObjects");
		{
			VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/BaseFloorObjects.obj");
			obj9.emplace<RendererComponent>().SetMesh(vao).SetMaterial(baselevelObjectsMat);
			obj9.get<Transform>().SetLocalPosition(0.0f, 0.0f, 0.0f);
			obj9.get<Transform>().SetLocalRotation(90.0f, 0.0f, 0.0f);
			BehaviourBinding::BindDisabled<SimpleMoveBehaviour>(obj9);
		}

		// Create an object to be our camera
		GameObject cameraObject = scene->CreateEntity("Camera");
		{
			cameraObject.get<Transform>().SetLocalPosition(0, 3, 3).LookAt(glm::vec3(0, 0, 0));

			// We'll make our camera a component of the camera object
			Camera& camera = cameraObject.emplace<Camera>();// Camera::Create();
			camera.SetPosition(glm::vec3(-3, 3, 10));
			camera.SetUp(glm::vec3(0, 0, 1));
			camera.LookAt(glm::vec3(0));
			camera.SetFovDegrees(90.0f); // Set an initial FOV
			camera.SetOrthoHeight(3.0f);
			BehaviourBinding::Bind<CameraControlBehaviour>(cameraObject);
		}

		int width, height;
		glfwGetWindowSize(BackendHandler::window, &width, &height);

		GameObject gBufferObject = scene->CreateEntity("G Buffer");
		{
			gBuffer = &gBufferObject.emplace<GBuffer>();
			gBuffer->Init(width, height);
		}

		GameObject illuminationBufferObject = scene->CreateEntity("Illumination Buffer");
		{
			illuminationBuffer = &illuminationBufferObject.emplace<IlluminationBuffer>();
			illuminationBuffer->Init(width, height);
		}

		int shadowWidth = 4096;
		int shadowHeight = 4096;

		GameObject shadowBufferObject = scene->CreateEntity("Shadow Buffer");
		{
			shadowBuffer = &shadowBufferObject.emplace<Framebuffer>();
			shadowBuffer->AddDepthTarget();
			shadowBuffer->Init(shadowWidth, shadowHeight);
		}

		GameObject framebufferObject = scene->CreateEntity("Basic Effect");
		{
			basicEffect = &framebufferObject.emplace<PostEffect>();
			basicEffect->Init(width, height);
		}
		effects.push_back(basicEffect);

		GameObject sepiaEffectObject = scene->CreateEntity("Sepia Effect");
		{
			sepiaEffect = &sepiaEffectObject.emplace<SepiaEffect>();
			sepiaEffect->Init(width, height);
		}
		effects.push_back(sepiaEffect);

		GameObject greyscaleEffectObject = scene->CreateEntity("Greyscale Effect");
		{
			greyscaleEffect = &greyscaleEffectObject.emplace<GreyscaleEffect>();
			greyscaleEffect->Init(width, height);
		}
		effects.push_back(greyscaleEffect);
		
		GameObject colorCorrectEffectObject = scene->CreateEntity("Greyscale Effect");
		{
			colorCorrectEffect = &colorCorrectEffectObject.emplace<ColorCorrectEffect>();
			colorCorrectEffect->Init(width, height);
		}
		effects.push_back(colorCorrectEffect);

		GameObject bloomEffectObject = scene->CreateEntity("Bloom Effect");
		{
			bloomEffect = &bloomEffectObject.emplace<BloomEffect>();
			bloomEffect->Init(width, height);
		}
		effects.push_back(bloomEffect);

		GameObject filmGrainEffectObject = scene->CreateEntity("Film Grain Effect");
		{
			filmGrainEffect = &filmGrainEffectObject.emplace<FilmGrainEffect>();
			filmGrainEffect->Init(width, height);
		}
		effects.push_back(filmGrainEffect);

		GameObject pixelateEffectObject = scene->CreateEntity("Pixelate Effect");
		{
			pixelateEffect = &pixelateEffectObject.emplace<PixelateEffect>();
			pixelateEffect->Init(width, height);
		}
		effects.push_back(pixelateEffect);

		#pragma endregion 
		//////////////////////////////////////////////////////////////////////////////////////////

		/////////////////////////////////// SKYBOX ///////////////////////////////////////////////
		//{
			// Load our shaders
			Shader::sptr skybox = std::make_shared<Shader>();
			skybox->LoadShaderPartFromFile("shaders/skybox-shader.vert.glsl", GL_VERTEX_SHADER);
			skybox->LoadShaderPartFromFile("shaders/skybox-shader.frag.glsl", GL_FRAGMENT_SHADER);
			skybox->Link();

			ShaderMaterial::sptr skyboxMat = ShaderMaterial::Create();
			skyboxMat->Shader = skybox;  
			skyboxMat->Set("s_Environment", environmentMap);
			skyboxMat->Set("u_EnvironmentRotation", glm::mat3(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1, 0, 0))));
			skyboxMat->RenderLayer = 100;

			MeshBuilder<VertexPosNormTexCol> mesh;
			MeshFactory::AddIcoSphere(mesh, glm::vec3(0.0f), 1.0f);
			MeshFactory::InvertFaces(mesh);
			VertexArrayObject::sptr meshVao = mesh.Bake();
			
			GameObject skyboxObj = scene->CreateEntity("skybox");  
			skyboxObj.get<Transform>().SetLocalPosition(0.0f, 0.0f, 0.0f);
			//skyboxObj.get_or_emplace<RendererComponent>().SetMesh(meshVao).SetMaterial(skyboxMat).SetCastShadow(false);
		//}
		////////////////////////////////////////////////////////////////////////////////////////

		// We'll use a vector to store all our key press events for now (this should probably be a behaviour eventually)
		std::vector<KeyPressWatcher> keyToggles;
		{
			// This is an example of a key press handling helper. Look at InputHelpers.h an .cpp to see
			// how this is implemented. Note that the ampersand here is capturing the variables within
			// the scope. If you wanted to do some method on the class, your best bet would be to give it a method and
			// use std::bind
			keyToggles.emplace_back(GLFW_KEY_T, [&]() { cameraObject.get<Camera>().ToggleOrtho(); });

			controllables.push_back(obj2);

			keyToggles.emplace_back(GLFW_KEY_KP_ADD, [&]() {
				BehaviourBinding::Get<SimpleMoveBehaviour>(controllables[selectedVao])->Enabled = false;
				selectedVao++;
				if (selectedVao >= controllables.size())
					selectedVao = 0;
				BehaviourBinding::Get<SimpleMoveBehaviour>(controllables[selectedVao])->Enabled = true;
				});
			keyToggles.emplace_back(GLFW_KEY_KP_SUBTRACT, [&]() {
				BehaviourBinding::Get<SimpleMoveBehaviour>(controllables[selectedVao])->Enabled = false;
				selectedVao--;
				if (selectedVao < 0)
					selectedVao = controllables.size() - 1;
				BehaviourBinding::Get<SimpleMoveBehaviour>(controllables[selectedVao])->Enabled = true;
				});

			keyToggles.emplace_back(GLFW_KEY_Y, [&]() {
				auto behaviour = BehaviourBinding::Get<SimpleMoveBehaviour>(controllables[selectedVao]);
				behaviour->Relative = !behaviour->Relative;
				});
		}

		// Initialize our timing instance and grab a reference for our use
		Timing& time = Timing::Instance();
		time.LastFrame = glfwGetTime();

		///// Game loop /////
		while (!glfwWindowShouldClose(BackendHandler::window)) {
			glfwPollEvents();

			// Update the timing
			time.CurrentFrame = glfwGetTime();
			time.DeltaTime = static_cast<float>(time.CurrentFrame - time.LastFrame);

			time.DeltaTime = time.DeltaTime > 1.0f ? 1.0f : time.DeltaTime;

			// Update our FPS tracker data
			fpsBuffer[frameIx] = 1.0f / time.DeltaTime;
			frameIx++;
			if (frameIx >= 128)
				frameIx = 0;

			// We'll make sure our UI isn't focused before we start handling input for our game
			if (!ImGui::IsAnyWindowFocused()) {
				// We need to poll our key watchers so they can do their logic with the GLFW state
				// Note that since we want to make sure we don't copy our key handlers, we need a const
				// reference!
				for (const KeyPressWatcher& watcher : keyToggles) {
					watcher.Poll(BackendHandler::window);
				}
			}

			// Iterate over all the behaviour binding components
			scene->Registry().view<BehaviourBinding>().each([&](entt::entity entity, BehaviourBinding& binding) {
				// Iterate over all the behaviour scripts attached to the entity, and update them in sequence (if enabled)
				for (const auto& behaviour : binding.Behaviours) {
					if (behaviour->Enabled) {
						behaviour->Update(entt::handle(scene->Registry(), entity));
					}
				}
			});

			// Clear the screen
			basicEffect->Clear();
			/*greyscaleEffect->Clear();
			sepiaEffect->Clear();*/
			for (int i = 0; i < effects.size(); i++)
			{
				effects[i]->Clear();
			}
			shadowBuffer->Clear();
			gBuffer->Clear();
			illuminationBuffer->Clear();

			glClearColor(1.0f, 1.0f, 1.0f, 0.3f);
			glEnable(GL_DEPTH_TEST);
			glClearDepth(1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Update all world matrices for this frame
			scene->Registry().view<Transform>().each([](entt::entity entity, Transform& t) {
				t.UpdateWorldMatrix();
			});

			// Grab out camera info from the camera object
			Transform& camTransform = cameraObject.get<Transform>();
			glm::mat4 view = glm::inverse(camTransform.LocalTransform());
			glm::mat4 projection = cameraObject.get<Camera>().GetProjection();
			glm::mat4 viewProjection = projection * view;

			//Set up light space matrix
			glm::mat4 lightProjectionMatrix = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, -30.0f, 30.0f);
			//glm::mat4 lightViewMatrix = glm::lookAt(glm::vec3(-theSun._lightDirection), glm::vec3(), glm::vec3(0.0f, 0.0f, 1.0f));
			glm::mat4 lightViewMatrix = glm::lookAt(glm::vec3(-illuminationBuffer->GetSunRef()._lightDirection), glm::vec3(), glm::vec3(0.0f, 0.0f, 1.0f));
			glm::mat4 lightSpaceViewProj = lightProjectionMatrix * lightViewMatrix;

			//Set shadow stuff
			illuminationBuffer->SetLightSpaceViewProj(lightSpaceViewProj);
			glm::vec3 camPos = glm::inverse(view) * glm::vec4(0, 0, 0, 1);
			illuminationBuffer->SetCamPos(camPos);

			// Sort the renderers by shader and material, we will go for a minimizing context switches approach here,
			// but you could for instance sort front to back to optimize for fill rate if you have intensive fragment shaders
			renderGroup.sort<RendererComponent>([](const RendererComponent& l, const RendererComponent& r) {
				// Sort by render layer first, higher numbers get drawn last
				if (l.Material->RenderLayer < r.Material->RenderLayer) return true;
				if (l.Material->RenderLayer > r.Material->RenderLayer) return false;

				// Sort by shader pointer next (so materials using the same shader run sequentially where possible)
				if (l.Material->Shader < r.Material->Shader) return true;
				if (l.Material->Shader > r.Material->Shader) return false;

				// Sort by material pointer last (so we can minimize switching between materials)
				if (l.Material < r.Material) return true;
				if (l.Material > r.Material) return false;

				return false;
			});

			// Start by assuming no shader or material is applied
			Shader::sptr current = nullptr;
			ShaderMaterial::sptr currentMat = nullptr;

			glViewport(0, 0, shadowWidth, shadowHeight);
			shadowBuffer->Bind();

			renderGroup.each([&](entt::entity e, RendererComponent& renderer, Transform& transform) {
				// Render the mesh
				if (renderer.CastShadows)
				{
					BackendHandler::RenderVAO(simpleDepthShader, renderer.Mesh, viewProjection, transform, lightSpaceViewProj);
				}
			});

			shadowBuffer->Unbind();

			glfwGetWindowSize(BackendHandler::window, &width, &height);

			glViewport(0, 0, width, height);
			//basicEffect->BindBuffer(0);
			gBuffer->Bind();
			// Iterate over the render group components and draw them
			renderGroup.each([&](entt::entity e, RendererComponent& renderer, Transform& transform) {
				// If the shader has changed, set up it's uniforms
				if (current != renderer.Material->Shader) {
					current = renderer.Material->Shader;
					current->Bind();
					BackendHandler::SetupShaderForFrame(current, view, projection);
				}
				// If the material has changed, apply it
				if (currentMat != renderer.Material) {
					currentMat = renderer.Material;
					currentMat->Apply();
				}

				//shadowBuffer->BindDepthAsTexture(30);
				// Render the mesh
				BackendHandler::RenderVAO(renderer.Material->Shader, renderer.Mesh, viewProjection, transform, lightSpaceViewProj);			
			});

			//shadowBuffer->UnbindTexture(30);
			//basicEffect->UnbindBuffer();
			gBuffer->Unbind();

			illuminationBuffer->BindBuffer(0);

			skybox->Bind();
			BackendHandler::SetupShaderForFrame(skybox, view, projection);
			skyboxMat->Apply();
			BackendHandler::RenderVAO(skybox, meshVao, viewProjection, skyboxObj.get<Transform>(), lightSpaceViewProj);
			skybox->UnBind();

			illuminationBuffer->UnbindBuffer();

			shadowBuffer->BindDepthAsTexture(30);

			illuminationBuffer->ApplyEffect(gBuffer);

			shadowBuffer->UnbindTexture(30);

			/*if (drawGBuffer)
			{
				gBuffer->DrawBuffersToScreen();
			}
			else if (drawIllumBuffer)
			{
				illuminationBuffer->DrawIllumBuffer();
			}
			else
			{
				effects[activeEffect]->ApplyEffect(illuminationBuffer);
				effects[activeEffect]->DrawToScreen();
			}*/

			if (drawAlbedoBuffer)
			{
				gBuffer->DrawAlbedoBuffer();
			}
			else if (drawNormalsBuffer)
			{
				gBuffer->DrawNormalsBuffer();			
			}
			else if (drawPositionsBuffer)
			{
				gBuffer->DrawPositionBuffer();			
			}
			else if (drawSpecularBuffer)
			{
				gBuffer->DrawSpecularBuffer();
			}
			else if (drawGBuffer)
			{
				gBuffer->DrawBuffersToScreen();
			}
			else if (drawIllumBuffer)
			{
				illuminationBuffer->DrawIllumBuffer();
			}
			else
			{
				effects[activeEffect]->ApplyEffect(illuminationBuffer);
				effects[activeEffect]->DrawToScreen();
			}
			
			// Draw our ImGui content
			BackendHandler::RenderImGui();

			scene->Poll();
			glfwSwapBuffers(BackendHandler::window);
			time.LastFrame = time.CurrentFrame;
		}
		//directionalLightBuffer.Unbind(0);

		// Nullify scene so that we can release references
		Application::Instance().ActiveScene = nullptr;
		//Clean up the environment generator so we can release references
		//EnvironmentGenerator::CleanUpPointers();
		BackendHandler::ShutdownImGui();
	}	

	// Clean up the toolkit logger so we don't leak memory
	Logger::Uninitialize();
	return 0;
}