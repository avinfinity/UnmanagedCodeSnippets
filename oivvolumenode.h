#ifndef __IMT_OIVVOLUMENODE__
#define __IMT_OIVVOLUMENODE__
#include "QOpenGLFunctions"

#include "trackballcamera.h"


class SoVolumeData;
class SoSeparator;
class SoOrthographicCamera;
class SbViewportRegion;
class SoTransform;
class SoMaterial;
class SoSceneManager;
class SoTransferFunction;
class SoDirectionalLight;
class SoVolumeIsosurface;
class SoInteractiveComplexity;
class SoSwitch;
class SoVolumeRender;
class SoVolumeRenderingQuality;

namespace imt{

	namespace volume{



		class OivVolumeNode : public QOpenGLFunctions
		{


		public:

			/// <summary>
			/// Eine neue Instanz erstellen.
			/// </summary>
			OivVolumeNode(SoVolumeData* volumeData, TrackBallCamera *camera );



			/// <summary>
			/// Die Instanz zerstören und alle Daten freigeben.
			/// </summary>
			virtual ~OivVolumeNode();


			//void init();


			/// <summary>
			/// Der Knoten zeichnet sich.
			/// </summary>
			/// <param name="mode">zu benutzender Zeichenmodus</param>
			void MemoryInit();

			/// <summary>
			/// Der Knoten zeichnet sich.
			/// </summary>
			/// <param name="mode">zu benutzender Zeichenmodus</param>
			void Render();//(const Zeiss::IMT::CAD::Viewer::GLRenderMode& mode);

			/// <summary>
			/// Einen 3D-Punkt mit Bildschirmkoordinaten extrahieren.
			/// </summary>
			/// <param name="camera">die zugehörige OpenGL Kamera</param>
			/// <param name="x">die horizontale Bildschirmkoordinate, relativ zum OpenGL Fenster</param>
			/// <param name="y">die vertikale Bildschirmkoordinate, relativ zum OpenGL Fenster</param>
			/// <param name="p">der getroffene 3D-Punkt</param>
			/// <param name="d">die Direction der getroffenen Fläche</param>
			/// <returns>true wenn der Pickvorgang erfolgreich war, flase wenn nicht</returns>
			//bool Pick(Zeiss::IMT::CAD::Viewer::GLCamera const& camera, int x, int y, Zeiss::IMT::Numerics::Position3d& p, Zeiss::IMT::Numerics::Direction3d& d);


			/// <summary>
			/// Den verwalteten Root-Knoten abrufen.
			/// </summary>
			SoSeparator& RequestRoot();

			/// <summary>
			/// Den verwalteten SoTransferFunction-Knoten abrufen.
			/// </summary>
			SoTransferFunction& RequestTransferFunction();

			/// <summary>
			/// Den verwalteten SoMaterial-Knoten abrufen.
			/// </summary>
			SoMaterial& RequestMaterial();

			/// <summary>
			/// Request the managed SoSceneManager.
			/// </summary>
			SoSceneManager& RequestSceneManager();

			/// <summary>
			/// Set the iso value for picking.
			/// </summary>
			void SetPickingIsoValue(int isoValue);

			/// <summary>
			/// Flag determining if the node is currently rendering.
			/// </summary>
			bool IsRendering();

			/// <summary>
			/// Flag determining if the node was refreshed.
			/// </summary>
			bool IsRefreshed();

			bool _InitMode;



		protected:

			SoSeparator* _Root;
			SoSeparator* _PickRoot;
			SoOrthographicCamera* _Camera;
			SoVolumeData* _VolumeData;
			SoTransform* _Transform;
			SoTransferFunction* _TransferFunction;
			SoTransferFunction* _PickTransferFunction;
			SoMaterial* _Material;
			SoSceneManager* _SceneManager;
			SoSceneManager* _PickSceneManager;

			SoDirectionalLight* _DirectionalLight;
			SoSwitch* _RenderSwitch;
			SoSwitch* _SliceSwitch;
			SoSwitch* _RenderQualitySwitch;
			SoVolumeIsosurface* _IsosurfaceRenderer;
			SoInteractiveComplexity* _Complexity;
			SoVolumeRenderingQuality* _RenderingQuality;
			SoVolumeRender* _VolumeRender;


			int _PickingIsoValue;
			bool _IsRefreshed;
			int _ChachedRendererActivation;
			float* _CachedDiffuseColor;
			float* _CachedColorMap;
			bool _UseColorPick;
			bool _Disposing;
			static bool _IsRendering;
			unsigned short _InstanceId;

			TrackBallCamera *_TrackBallCamera;

			/// <summary>
			/// Die Bounding Box des Volumens berechnen.
			/// </summary>
			/// <param name="box">die vorhandene Bounding Box, zu welcher die Bounding Box des Volumens hinzu gerechnet wird</param>
			/// <param name="alignment">die zugehörige Ausrichtung der Bounding Box des Volumens</param>
			void CalculateBoundingBox(TrackBallCamera *camera) ;//(Zeiss::IMT::CAD::Viewer::GLBoundingBox& box, const Zeiss::IMT::CAD::Viewer::GLAlignment* alignment) const;

			/// <summary>
			/// Die Bounding Box des Volumens berechnen.
			/// </summary>
			/// <param name="box">die vorhandene Bounding Box, zu welcher die Bounding Box des Volumens hinzu gerechnet wird</param>
			/// <param name="alignment">die zugehörige Ausrichtung der Bounding Box des Volumens</param>
			void Update(TrackBallCamera *camera, SbViewportRegion& viewPort);//(Zeiss::IMT::CAD::Viewer::GLCamera const& camera, SbViewportRegion& viewPort);

			/// <summary>
			/// Einen Quader mit der Seitenlänge von eins zeichnen.
			/// </summary>
			void DrawBox();

			/// <summary>
			/// Im Volumen über VolumeViz Funktionalität picken.
			/// </summary>
			/// <param name="x">die horizontale Bildschirmkoordinate, relativ zum OpenGL Fenster</param>
			/// <param name="y">die vertikale Bildschirmkoordinate, relativ zum OpenGL Fenster</param>
			/// <param name="p">der getroffene 3D-Punkt</param>
			/// <returns>true wenn der Pickvorgang erfolgreich war, flase wenn nicht</returns>
			//bool PickBase(double x, double y, Zeiss::IMT::Numerics::Position3d& p);

			/// <summary>
			/// Returns a filled bounding box node. 
			/// This box is later rendered to enable the pick functionality for points.
			/// </summary>
			/// <param name="box">Bounding box for the volume data</param>
			/// <returns>Seperator node which contains the volume bounding box</returns>
			SoSeparator *makeBBox(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax);

			/// <summary>
			/// Den VSG Scenen Grafen initalisieren.
			/// </summary>
			virtual void InitializeSceneGraph();

			/// <summary>
			/// Den VSG Scenen Grafen zerstören.
			/// </summary>
			virtual void TerminateSceneGraph();

			/// <summary>
			/// Prepares the volume rendering for picking mode.
			/// </summary>
			void StartPickRendering();

			/// <summary>
			/// Restores the volume rendering settings for normal rendering.
			/// </summary>
			void FinishPickRendering();

			/// <summary>
			/// Renders the scene based upon the GLRenderMode.
			/// </summary>
			virtual void OnRendering(); //const Zeiss::IMT::CAD::Viewer::GLRenderMode& mode


			SoSwitch& RequestRenderSwitch();

			/// <summary>
			/// Den verwalteten SoSwitch-Knoten, welcher die zu benutzenden Schnitte verwaltet, abrufen.
			/// </summary>
			SoSwitch& RequestSliceSwitch();

			/// <summary>
			/// Den verwalteten SoSwitch-Knoten, welcher die Einstellunger der Volumen-Render-Qualität verwaltet, abrufen.
			/// </summary>
			SoSwitch& RequestRenderQualitySwitch();

			/// <summary>
			/// Den verwalteten SoVolumeIsosurface-Knoten, welcher die Isosurface Rendering-Einstellungen verwaltet, abrufen.
			/// </summary>
			SoVolumeIsosurface& RequestVolumeIsosurface();

			/// <summary>
			/// Den verwalteten SoInteractiveComplexity-Knoten, welcher die Rendering-Komplexität beim Interagieren verwaltet, abrufen.
			/// </summary>
			SoInteractiveComplexity& RequestInteractiveComplexity();

			/// <summary>
			/// Request the volume renderer.
			/// </summary>
			SoVolumeRender& RequestVolumeRender();

			/// <summary>
			/// Request the renderer quality.
			/// </summary>
			SoVolumeRenderingQuality& RequestRenderQuality();





		};




	}

}


#endif