#include "oivvolumenode.h"


#include <Inventor/SoSceneManager.h>
#include <Inventor/SoPickedPoint.h>
#include <Inventor/elements/SoGLLazyElement.h>
#include <Inventor/devices/SoGLContext.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/nodes/SoOrthographicCamera.h>
#include <Inventor/nodes/SoTransform.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoRayPickAction.h>
#include <VolumeViz/details/SoVolumeRenderDetail.h>
#include <VolumeViz/nodes/SoVolumeData.h>
#include <VolumeViz/nodes/SoVolumeIsosurface.h>
#include <VolumeViz/nodes/SoVolumeRender.h>
#include <VolumeViz/nodes/SoVolumeRenderingQuality.h>
#include <Inventor\nodes\SoComplexity.h>
#include <Inventor\nodes\SoLightModel.h>
#include <Inventor\nodes\SoDrawStyle.h>
#include <Inventor\nodes\SoPickStyle.h>
#include <Inventor\nodes\SoVertexProperty.h>
#include <Inventor\nodes\SoIndexedLineSet.h>
#include <Inventor\nodes\SoIndexedFaceSet.h>
#include <VolumeViz/nodes/SoVolumeIsosurface.h>
#include <VolumeViz/nodes/SoVolumeRender.h> 
#include <VolumeViz/nodes/SoVolumeRenderingQuality.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/nodes/SoInteractiveComplexity.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor\nodes\SoComplexity.h>


void renderCB(void *userData, SoSceneManager * /*mgr*/)
{
	//Zeiss::IMT::NG::Metrotom::Data::Entities::Logger::Debug("OivVolumeNode", "Rendercallback is called by VSG...");
	//Zeiss::IMT::NG::Metrotom::VSGWrapper::OivVolumeNode* renderNode = static_cast<Zeiss::IMT::NG::Metrotom::VSGWrapper::OivVolumeNode*>(userData);
	//if (renderNode->IsRendering() || renderNode->_InitMode)
	//	return;

	//Zeiss::IMT::NG::Metrotom::Data::Entities::Logger::Debug("OivVolumeNode", "Publishing a refresh...");

	//IRenderRef^ renderRef = gcnew NodeRenderRef(gcnew Zeiss::IMT::CAD::GLNodeRef(renderNode));
	//renderRef->Invalidate();

	//Zeiss::IMT::NG::Metrotom::Data::Entities::Logger::Debug("OivVolumeNode", "Rendercallback is finished!");

	//std::cout << " render cb " << std::endl;
}


namespace imt
{

				namespace volume
				{



					bool OivVolumeNode::_IsRendering = false;
					//unsigned short OivVolumeNode::_InstanceCount = 1;


					OivVolumeNode::OivVolumeNode(SoVolumeData* volumeData, TrackBallCamera *camera) :
						//GLShape(),
						_Root(new SoSeparator),
						_VolumeData(volumeData),
						_Camera(new SoOrthographicCamera),
						_Transform(new SoTransform),
						_TransferFunction(new SoTransferFunction),
						_PickTransferFunction(new SoTransferFunction),
						_Material(new SoMaterial),
						_PickingIsoValue(0),
						_IsRefreshed(false),
						_CachedDiffuseColor(nullptr),
						_CachedColorMap(nullptr),
						_ChachedRendererActivation(-1),
						_UseColorPick(false),
						_Disposing(false), _InitMode(false),
						_DirectionalLight(new SoDirectionalLight),
						_Complexity(0),
						_RenderSwitch(0),
						_RenderQualitySwitch(0),
						_IsosurfaceRenderer(0),
						_SliceSwitch(0),
						_VolumeRender(0),
						_RenderingQuality(0),
						_TrackBallCamera(camera)
					{

						initializeOpenGLFunctions();

						//_InstanceId = _InstanceCount++;
						SoMaterial* pickMaterial = new SoMaterial();
						pickMaterial->setName("PickMaterial");//NoResourceText
						pickMaterial->diffuseColor.setValue(1.0f, 1.0f, 1.0f);

						_PickRoot = new SoSeparator();
						_PickRoot->ref();

						_PickRoot->addChild(_Camera);
						_PickRoot->addChild(_Transform);
						_PickRoot->addChild(_VolumeData);
						_PickRoot->addChild(_PickTransferFunction);

						SoMaterial* colorPickMaterial = new SoMaterial();
						colorPickMaterial->ambientColor.setValue(1.0f, 1.0f, 1.0f);
						colorPickMaterial->diffuseColor.setValue(1.0f, 1.0f, 1.0f);
						colorPickMaterial->emissiveColor.setValue(1.0f, 1.0f, 1.0f);
						colorPickMaterial->shininess.setValue(0.2f);
						_PickRoot->addChild(colorPickMaterial);

						_PickSceneManager = new SoSceneManager();
						_PickSceneManager->setSceneGraph(_PickRoot);
						_PickSceneManager->setRenderCallback(renderCB, this);
						_PickSceneManager->getGLRenderAction()->setTransparencyType(SoGLRenderAction::DELAYED_BLEND);
						_PickSceneManager->getGLRenderAction()->setDelayedObjDepthWrite(TRUE);
						_PickSceneManager->activate();
						_PickSceneManager->scheduleRedraw();

						_Camera->setName("Camera");//NoResourceText
						_Transform->setName("Transform");//NoResourceText
						_VolumeData->setName("VolumeData");//NoResourceText
						_TransferFunction->setName("TransferFunction");//NoResourceText

						_Root->ref();
						_Root->addChild(_Camera);
						_Root->addChild(_Transform);
						_Root->addChild(_VolumeData);
						_Root->addChild(_TransferFunction);
						_Root->addChild(pickMaterial);

						_Material->setName("MainMaterial");//NoResourceText
						_Material->ambientColor.setValue(0.0f, 0.0f, 0.0f);
						_Material->diffuseColor.set1Value(0, 0.8f, 0.8f, 0.8f);
						_Material->specularColor.setValue(0.2f, 0.2f, 0.2f);
						_Material->emissiveColor.setValue(1.0f, 1.0f, 1.0f);
						_Material->shininess.setValue(0.2f);
						_Root->addChild(_Material);

						_SceneManager = new SoSceneManager();
						_SceneManager->setSceneGraph(_Root);
						_SceneManager->setRenderCallback(renderCB, this);
						_SceneManager->getGLRenderAction()->setTransparencyType(SoGLRenderAction::DELAYED_BLEND);
						_SceneManager->getGLRenderAction()->setDelayedObjDepthWrite(TRUE);
						_SceneManager->activate();
						_SceneManager->scheduleRedraw();


						InitializeSceneGraph();
						//SetAlignment(new GLAlignment());
						//SetAppearance(new GLAppearance());

						//_Logger.open("renderstamp.txt", std::ios::out);


						// Use a predefined colorMap with the SoTransferFunction
						//SoTransferFunction* pTransFunc = new SoTransferFunction;
						//pTransFunc->predefColorMap = SoTransferFunction::NONE;//NONE;//NONE; //STANDARD;//

						//pTransFunc->minValue = minVal;
						//pTransFunc->maxValue = maxVal;
						_TransferFunction->predefColorMap = SoTransferFunction::NONE;
						_TransferFunction->minValue.setValue(0);
						_TransferFunction->maxValue.setValue(65535);//

						std::fstream colorMapReader;

						std::vector<float> interpolatedMap(65536 * 4);

						colorMapReader.open("C:/Data/ZxoData/separated_part_7_colormap.dat", std::ios::in | std::ios::binary);

						colorMapReader.read((char*)interpolatedMap.data(), 65536 * 4 * sizeof(float));

						colorMapReader.close();

						_TransferFunction->colorMap.setValues(0, 65536 * 4, interpolatedMap.data());
						_TransferFunction->enableNotify(TRUE);
						_TransferFunction->touch();

					}

					OivVolumeNode::~OivVolumeNode()
					{
						_SceneManager->setRenderCallback(NULL, NULL);
						_PickSceneManager->setRenderCallback(NULL, NULL);

						_Disposing = true;

						_PickRoot->removeAllChildren();
						_PickRoot->unref();
						_PickRoot = 0;

						_Root->removeAllChildren();
						_Root->unref();
						_Root = 0;

						_Camera = 0;
						_Transform = 0;
						_VolumeData = 0;
						_TransferFunction = 0;

						delete _SceneManager;
						_SceneManager = 0;

						delete _PickSceneManager;
						_PickSceneManager = 0;

						if (_CachedColorMap != nullptr)
						{
							delete[] _CachedColorMap;
							_CachedColorMap = nullptr;
						}

						if (_CachedDiffuseColor != nullptr)
						{
							delete[] _CachedDiffuseColor;
							_CachedDiffuseColor = nullptr;
						}
					}


					void OivVolumeNode::Render()//(const GLRenderMode& mode)
					{
						OnRendering();// (mode);
					}

					void OivVolumeNode::OnRendering() //const GLRenderMode& mode
					{
						/*if (!IsVisible() || _Disposing)
							return;*/

						_IsRendering = true;


						bool isSelectable = true;// IsSelectable() == smSelectable;
						bool isNormalSelectionMode = true;// mode.Type() == rmSelect;
						bool isColorSelectionMode = true;//mode.Type() == rmColorSelect;
						bool isNormalPickRendering = (!_IsRefreshed || !_UseColorPick || !isSelectable) && isColorSelectionMode || isNormalSelectionMode;
						bool isColorPickRendering = false;// _IsRefreshed && _UseColorPick && isColorSelectionMode && isSelectable;

						/*if (isColorPickRendering)
							StartPickRendering();*/

						//PreRenderCommands(mode);

						//if (!isNormalPickRendering)
						{
							glPushAttrib(GL_ALL_ATTRIB_BITS);
							glDisable(GL_ALPHA_TEST);

							glMatrixMode(GL_PROJECTION);
							glPushMatrix();
							glLoadIdentity();

							glMatrixMode(GL_MODELVIEW);
							glPushMatrix();
							glLoadIdentity();

							// provide Open Inventor a clean OpenGL context
							bool isNormalArrayEnabled = glIsEnabled(GL_NORMAL_ARRAY);
							bool isColorArrayEnabled = glIsEnabled(GL_COLOR_ARRAY);
							bool isVertexArrayEnabled = glIsEnabled(GL_VERTEX_ARRAY);

							if (isNormalArrayEnabled)
								glDisableClientState(GL_NORMAL_ARRAY);

							if (isColorArrayEnabled)
								glDisableClientState(GL_COLOR_ARRAY);

							if (isVertexArrayEnabled)
								glDisableClientState(GL_VERTEX_ARRAY);

							SbViewportRegion viewPort;

							Update(_TrackBallCamera, viewPort);//(*GLCamera::ActiveCamera(), viewPort);


							// get the current app OpenGL context
							SoGLContext* curglContext = SoGLContext::getCurrent(true);
							if (curglContext != nullptr)
							{
								curglContext->bind();

								//VSG bug #36096: 
								//OIV needs to have FrameBufferExtensions::GL_FRAMEBUFFER instead of FrameBufferExtensions::GL_DRAW_FRAMEBUFFER

								// Render the Open Inventor scene
								//if (isColorPickRendering)
								//{
								//	_PickSceneManager->setViewportRegion(viewPort);
								//	_PickSceneManager->render(false, false);
								//}
								//else
								//{

								//std::cout << " rendering scene manager " << std::endl;
									_SceneManager->setViewportRegion(viewPort);
									_SceneManager->render(false, false);

									DrawBox();
								//}

								curglContext->unbind();
								_IsRefreshed = true;
							}

							// reset the client states to get the state as before Open Inventor
							if (isNormalArrayEnabled)
								glEnableClientState(GL_NORMAL_ARRAY);

							if (isColorArrayEnabled)
								glEnableClientState(GL_COLOR_ARRAY);

							if (isVertexArrayEnabled)
								glEnableClientState(GL_VERTEX_ARRAY);

							glMatrixMode(GL_PROJECTION);
							glPopMatrix();

							glMatrixMode(GL_MODELVIEW);
							glPopMatrix();

							glPopAttrib();
						}

						//PostRenderCommands(mode);

						//if (isColorPickRendering)
						//	FinishPickRendering();

						_IsRendering = false;

					}

					//bool OivVolumeNode::Pick(GLCamera const& camera, int x, int y, Position3d& p, Direction3d& d)
					//{
					//	bool determineNormalWithAdditionalPoints = d[2] != -UnitVector3d::UnitVectorZ;

					//	Position3d pickedPoint;
					//	bool success = PickBase(x, y, pickedPoint);

					//	if (!success)
					//	{
					//		return false;
					//	}

					//	Vector3d n;

					//	// Determine default normal
					//	Vector3d lookingDir = camera.Eye() - camera.Target();
					//	if (Norm(lookingDir) > Zeiss::IMT::Math::MathConstants::SqrtEps)
					//		n = Normalize(lookingDir);
					//	else
					//		n = Vector3d(0.0, 0.0, 1.0);

					//	// Determine normal with additional points
					//	if (determineNormalWithAdditionalPoints)
					//	{
					//		//Vector3d distVect = pickedPoint - p;
					//		//double distance = Norm(distVect);
					//		//const double maxPickDistance = 5.0 * camera.UnitsPerPixel();

					//		// if the distance is higher as the max pick distance we've hit a point 
					//		// which is too far away from the color pickked point maybe we've picked
					//		// on a clipping plane, but we can use this information to calculate a 
					//		// picking normal
					//		//if (distance <= maxPickDistance)
					//		{
					//			// the found point is near enough to use and we can go on to pick four points
					//			// around it to calculate a normal
					//			Position3d point;
					//			std::vector<Position3d> pickedPoints;
					//			const int pickDisplacement = std::max(1, (int)ceil(0.5 / camera.UnitsPerPixel()));
					//			if (PickBase(x - pickDisplacement, y, point))
					//				pickedPoints.push_back(point);

					//			if (PickBase(x, y - pickDisplacement, point))
					//				pickedPoints.push_back(point);

					//			if (PickBase(x, y + pickDisplacement, point))
					//				pickedPoints.push_back(point);

					//			if (PickBase(x + pickDisplacement, y, point))
					//				pickedPoints.push_back(point);

					//			if (pickedPoints.size() >= 2)
					//				n = Cross(pickedPoints[0] - pickedPoint, pickedPoints[1] - pickedPoint);

					//			if (Norm(n) > Zeiss::IMT::Math::MathConstants::SqrtEps)
					//				n = Normalize(n);
					//		}
					//	}

					//	p = pickedPoint;
					//	d = Direction3d(Normalize(n));
					//	return true;
					//}

					//bool OivVolumeNode::PickBase(double x, double y, Position3d& p)
					//{
					//	SoSceneManager* sceneManager = _SceneManager;
					//	SoRayPickAction action = SoRayPickAction(sceneManager->getViewportRegion());
					//	action.setPoint(SbVec2s((short)x, (short)y));
					//	action.enableNormalsGeneration(FALSE);
					//	action.setRadius(10);
					//	action.apply(sceneManager->getSceneGraph());

					//	SoPickedPoint* result = action.getPickedPoint();
					//	if (!result)
					//		return false;

					//	SbVec3f objPos = result->getPoint();
					//	p = Position3d(objPos[0], objPos[1], objPos[2]);
					//	return true;
					//}

					void OivVolumeNode::Update(TrackBallCamera *camera, SbViewportRegion& viewPort)//(GLCamera const& camera, SbViewportRegion& viewPort)
					{

						auto& T =  camera->getModelViewProjectionMatrix();

						// Alignment
						//Transf3d const& T = _Alignment->AllMatrix();



						SbMatrix t(
							(float)T(0, 0), (float)T(1, 0), (float)T(2, 0), 0.0f,
							(float)T(0, 1), (float)T(1, 1), (float)T(2, 1), 0.0f,
							(float)T(0, 2), (float)T(1, 2), (float)T(2, 2), 0.0f,
							(float)T(0, 3), (float)T(1, 3), (float)T(2, 3), 1.0f);

						_Transform->setMatrix(t);

						//Eigen::Vector2cd eye(0, 0, 0);

						// Camera
						_Camera->position.setValue(0, 0, 0);//((float)camera.Eye()[0], (float)camera.Eye()[1], (float)camera.Eye()[2]);

						auto& Q = camera->getModelViewMatrix();

						//Direction3d Q(camera.ModelViewMatrix());

						/*SbMatrixd m(
							Q[0][0], Q[1][0], Q[2][0], 0,
							Q[0][1], Q[1][1], Q[2][1], 0,
							Q[0][2], Q[1][2], Q[2][2], 0,
							0, 0, 0, 1);*/


						//SbMatrixd(double a11, double a12, double a13, double a14,
						//	double a21, double a22, double a23, double a24,
						//	double a31, double a32, double a33, double a34,
						//	double a41, double a42, double a43, double a44);


						SbMatrixd m(
							Q(0, 0), Q(0, 1), Q(0, 2), Q(0, 3),
							Q(1, 0), Q(1, 1), Q(1, 2), Q(1, 3),
							Q(2, 0), Q(2, 1), Q(2, 2), Q(2, 3),
							Q(3, 0), Q(3, 1), Q(3, 2), Q(3, 3)
							);

						_Camera->orientation.setValue(SbRotationd(m));

						int w, h;

						camera->getDimensions(w, h);

						//std::cout << w << " " << h << " " << camera->nearPlane() << " " << camera->farPlane() << std::endl;

						_Camera->nearDistance = camera->nearPlane(); //(float)camera.ViewVolume().zMin();
						_Camera->farDistance = camera->farPlane(); //(float)camera.ViewVolume().zMax();

						int vpwidth = w;// camera.ViewPort()[2];
						int vpheight = h;// camera.ViewPort()[3];

						float yLength = 20;// h;// (float)camera.ViewVolume().yLength();
						float xLength = 20;// w;// (float)camera.ViewVolume().xLength();
						if (vpwidth >= vpheight)
						{
							_Camera->aspectRatio = xLength / yLength;
							_Camera->height = yLength;
						}
						else
						{
							_Camera->aspectRatio = yLength / xLength;
							_Camera->height = xLength;
						}

						// Viewport
						short viewPortPixelsLeft = 0;// (short)camera.ViewPort()[0];
						if (viewPortPixelsLeft < 0)
							viewPortPixelsLeft = 0;

						short viewPortPixelsBottom = 0;// (short)camera.ViewPort()[1];
						if (viewPortPixelsBottom < 0)
							viewPortPixelsBottom = 0;

						short viewPortPixelsWidth = w;//(short)camera.ViewPort()[2];
						if (viewPortPixelsWidth < 0)
							viewPortPixelsWidth = 0;

						short viewPortPixelsHeight = h;// (short)camera.ViewPort()[3];
						if (viewPortPixelsHeight < 0)
							viewPortPixelsHeight = 0;

						viewPort.setViewportPixels(viewPortPixelsLeft, viewPortPixelsBottom, viewPortPixelsWidth, viewPortPixelsHeight);
					}

					//void OivVolumeNode::CalculateBoundingBox(GLBoundingBox& box, const GLAlignment* alignment) const
					//{
					//	Position3d minPos(-1.0, -1.0, -1.0);
					//	Position3d maxPos(1.0, 1.0, 1.0);

					//	SbBox3f volumeBox = _VolumeData->extent.getValue();
					//	if (!volumeBox.isEmpty())
					//	{
					//		minPos = Position3d(volumeBox.getMin()[0], volumeBox.getMin()[1], volumeBox.getMin()[2]);
					//		maxPos = Position3d(volumeBox.getMax()[0], volumeBox.getMax()[1], volumeBox.getMax()[2]);
					//	}

					//	GLBoundingBox bb(minPos, maxPos);

					//	if (alignment != 0)
					//	{
					//		bb *= alignment->AllMatrix();
					//	}
					//	box += bb;
					//}

					void OivVolumeNode::DrawBox()
					{

#if 1
						auto volumeSize = _VolumeData->getVolumeSize();

						const double hl = volumeSize.getMax()[0] / 2;//1.0;
						const double hw = volumeSize.getMax()[1] / 2; //1.0;
						const double hh = volumeSize.getMax()[2] / 2; //1.0;

						const float pv[8 * 3] =
						{
							hl, -hw, -hh,
							hl, -hw, hh ,
							hl, hw, -hh ,
							hl, hw, hh,
							-hl, hw, -hh,
							-hl, hw, hh ,
							-hl, -hw, -hh ,
							-hl, -hw, hh };

						const float nv[6 * 3] =
						{
							0 , 0 , 1,
							0 , 0 , -1,
							0 , -1 , 0,
							0 , 1 , 0,
							1 , 0 , 0,
							-1 , 0 , 0 };

						const int planes[6][4] = {
							{ 1, 3, 5, 7 },
							{ 0, 6, 4, 2 },
							{ 0, 1, 7, 6 },
							{ 2, 4, 5, 3 },
							{ 0, 2, 3, 1 },
							{ 4, 6, 7, 5 } };

						glBegin(GL_QUADS);

						for (int i = 0; i < 6; ++i)
						{
							for (int j = 0; j < 4; ++j)
							{
								glNormal3d(nv[4 * i], nv[4 * i + 1], nv[4 * i + 2]);
								glVertex3d(pv[planes[i][j] * 3], pv[planes[i][j] * 3 + 1 ], pv[planes[i][j] * 3 + 2]);
							}
						}

						glEnd();
#endif
					}

					void OivVolumeNode::StartPickRendering()
					{

						// set the picking color in the transfer function
						if (_PickTransferFunction == nullptr)
							return;

						// read the needed picking color
						float color[4];
						glGetFloatv(GL_CURRENT_COLOR, color);

						//if (!IsNodeRequiresPickTransferFunctionUpdate(color))
						{
							_PickTransferFunction->enableNotify(FALSE);

							for (int i = 0; i < 256; i++)
							{
								_PickTransferFunction->colorMap.set1Value(4 * i + 0, color[0]);
								_PickTransferFunction->colorMap.set1Value(4 * i + 1, color[1]);
								_PickTransferFunction->colorMap.set1Value(4 * i + 2, color[2]);

								float alpha = _TransferFunction->colorMap[4 * (i * 256) + 3];
								if (alpha <= 0.0f)
									_PickTransferFunction->colorMap.set1Value(4 * i + 3, 0.0);
								else
									_PickTransferFunction->colorMap.set1Value(4 * i + 3, 1.0);
							}

							_PickTransferFunction->enableNotify(TRUE);
							_PickTransferFunction->touch();
						}
					}

					void OivVolumeNode::FinishPickRendering()
					{
					}

					//bool OivVolumeNode::IsNodeRequiresPickTransferFunctionUpdate(float* glPickColorOnView)
					//{
					//	bool isEqual = false;
					//	auto pickTransferFuncColor = _PickTransferFunction->colorMap.getValues(0);
					//	if (pickTransferFuncColor != nullptr)
					//	{
					//		float currentColors[4] = { pickTransferFuncColor[0], pickTransferFuncColor[1], pickTransferFuncColor[2], 0 };
					//		isEqual = Zeiss::IMT::Math::MathTools::Equal(currentColors[0], glPickColorOnView[0]) &&
					//			Zeiss::IMT::Math::MathTools::Equal(currentColors[1], glPickColorOnView[1]) &&
					//			Zeiss::IMT::Math::MathTools::Equal(currentColors[2], glPickColorOnView[2]);
					//		float aVal;
					//		for (int i = 0; i < 256; i++)
					//		{
					//			currentColors[3] = *_PickTransferFunction->colorMap.getValues(4 * i + 3);
					//			aVal = (_TransferFunction->colorMap[4 * (i * 256) + 3] <= 0.0f) ? 0.0f : 1.0f;

					//			if (!Zeiss::IMT::Math::MathTools::Equal(currentColors[3], aVal))
					//			{
					//				isEqual = false;
					//				break;
					//			}
					//		}
					//	}
					//	return isEqual;
					//}

					//bool OivVolumeNode::TryGetIdentifier(unsigned short& identifier) const
					//{
					//	identifier = _InstanceId;
					//	return true;
					//}


					SoSeparator& OivVolumeNode::RequestRoot()
					{
						return *_Root;
					}

					SoTransferFunction& OivVolumeNode::RequestTransferFunction()
					{
						return *_TransferFunction;
					}

					SoMaterial& OivVolumeNode::RequestMaterial()
					{
						return *_Material;
					}

					SoSceneManager& OivVolumeNode::RequestSceneManager()
					{
						return *_SceneManager;
					}

					void OivVolumeNode::SetPickingIsoValue(int isoValue)
					{
						_PickingIsoValue = isoValue;
					}

					bool OivVolumeNode::IsRendering()
					{
						return _IsRendering;
					}

					bool OivVolumeNode::IsRefreshed()
					{
						return _IsRefreshed;
					}



					void OivVolumeNode::InitializeSceneGraph()
					{
						SoVolumeRenderingQuality* renderingQuality = new SoVolumeRenderingQuality();
						renderingQuality->lightingModel = SoVolumeRenderingQuality::OPENGL;
						renderingQuality->interpolateOnMove = TRUE;
						_PickRoot->addChild(renderingQuality);

						SoComplexity* pickComplexity = new SoComplexity();
						pickComplexity->value = 0.0f;
						pickComplexity->textureQuality = 0.0f;
						_PickRoot->addChild(pickComplexity);

						SoVolumeRender* pickVolumeRender = new SoVolumeRender();
						pickVolumeRender->samplingAlignment = SoVolumeRender::BOUNDARY_ALIGNED;
						pickVolumeRender->subdivideTile = TRUE;
						_PickRoot->addChild(pickVolumeRender);

						_Root->insertChild(_DirectionalLight, 0);


						_Complexity = new SoInteractiveComplexity();
						_Complexity->setName("InteractionSettings");//NoResourceText
						_Complexity->fieldSettings.set1Value(0, "SoComplexity value 0.0 0.1");//NoResourceText
						_Root->insertChild(_Complexity, 0);

						_SliceSwitch = new SoSwitch();
						_SliceSwitch->setName("SliceSwitch");//NoResourceText
						_Root->addChild(_SliceSwitch);
						_PickRoot->insertChild(_SliceSwitch, 5);

						_RenderQualitySwitch = new SoSwitch();
						_RenderQualitySwitch->setName("RenderQualitySwitch");//NoResourceText
						_RenderQualitySwitch->whichChild = 0;
						_Root->addChild(_RenderQualitySwitch);
						_RenderingQuality = new SoVolumeRenderingQuality();
						_RenderingQuality->setName("RenderQuality");//NoResourceText
						_RenderingQuality->lightingModel = SoVolumeRenderingQuality::OPENGL;
						_RenderingQuality->deferredLighting = TRUE;
						_RenderingQuality->interpolateOnMove = TRUE;
						_RenderQualitySwitch->addChild(_RenderingQuality);

						SoComplexity* complexity = new SoComplexity();
						complexity->value = 0.1f;
						complexity->textureQuality = 0.1f;
						_Root->addChild(complexity);

						_RenderSwitch = new SoSwitch();
						_RenderSwitch->setName("RenderSwitch");//NoResourceText
						_Root->addChild(_RenderSwitch);

						_IsosurfaceRenderer = new SoVolumeIsosurface();
						_IsosurfaceRenderer->setName("IsosurfaceRenderer");//NoResourceText
						_IsosurfaceRenderer->interpolateOnMove = TRUE;
						_RenderSwitch->addChild(_IsosurfaceRenderer);
						_RenderSwitch->whichChild = -1;

						_VolumeRender = new SoVolumeRender();
						_VolumeRender->setName("VolumeRender");//NoResourceText
						_VolumeRender->samplingAlignment = SoVolumeRender::BOUNDARY_ALIGNED;
						// The artifact shown in Wuerfel_Knet_2x2binning (Isosurface) is now recorded as eBUG#5198 and it can be worked around by setting SoVolumeRender ::subdivideTile to FALSE.
						_VolumeRender->subdivideTile = TRUE;		// Parameter bleibt auf TRUE, wegen Performance
						_Root->addChild(_VolumeRender);




					}


					SoSwitch& OivVolumeNode::RequestRenderSwitch()
					{
						return *_RenderSwitch;
					}

					SoSwitch& OivVolumeNode::RequestSliceSwitch()
					{
						return *_SliceSwitch;
					}


					SoSwitch& OivVolumeNode::RequestRenderQualitySwitch()
					{
						return *_RenderQualitySwitch;
					}

					SoVolumeIsosurface& OivVolumeNode::RequestVolumeIsosurface()
					{
						return *_IsosurfaceRenderer;
					}

					SoInteractiveComplexity& OivVolumeNode::RequestInteractiveComplexity()
					{
						return *_Complexity;
					}

					SoVolumeRender& OivVolumeNode::RequestVolumeRender()
					{
						return *_VolumeRender;
					}

					SoVolumeRenderingQuality& OivVolumeNode::RequestRenderQuality()
					{
						return *_RenderingQuality;
					}

					void OivVolumeNode::TerminateSceneGraph()
					{
						_DirectionalLight = 0;
						_VolumeRender = 0;
						_Complexity = 0;
						_RenderSwitch = 0;
						_IsosurfaceRenderer = 0;
						_SliceSwitch = 0;
					}


	}
}


