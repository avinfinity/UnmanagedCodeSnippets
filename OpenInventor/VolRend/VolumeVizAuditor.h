#ifndef _VVIZ_AUDITOR
#define _VVIZ_AUDITOR

#include "volRend.h"

//---------------Auditor-------------------------------------------
class VolumeVizAuditor : public SoDialogAuditor
{
public:
  struct Data{
    SoTransferFunction* m_transferFunc;
  };
  Data m_data;
  VolumeVizAuditor();
  ~VolumeVizAuditor();
   void doDataChange( int selectedItem, const SbString& );

    // Action functions for cmap and data comboboxes
   void doCmapChange( int selectedItem, SoTransferFunction* );

private:

   void dialogCheckBox(SoDialogCheckBox* cpt);
   void dialogRealSlider(SoDialogRealSlider* cpt);
   void dialogComboBox(SoDialogComboBox* cpt);
   void dialogEditText(SoDialogEditText* cpt);
   void dialogPushButton(SoDialogPushButton* cpt);
   void dialogIntegerSlider(SoDialogIntegerSlider* cpt);
   void dialogRadioButtons (SoDialogRadioButtons *cpt) ;

   SbString userColorMapName;

   SbString userDataName;
   SbString userClipmapName;

   SbBool m_ROIManipNode;
   bool m_isInteractive;

   static void cmapFileDialogCB(void *data, SoXtFileSelectionDialog *dialog);
   static void dataFileDialogCB(void *data, SoXtFileSelectionDialog *dialog);
   static void settingsFileDialogCB(void *data, SoXtFileSelectionDialog *dialog);
   static void clipmapFileDialogCB(void *data, SoXtFileSelectionDialog *dialog);
   static void volumeClippingFileDialogCB(void *data, SoXtFileSelectionDialog *dialog);


   SoXtFileSelectionDialog *m_pCmapFileDialog;
   SoXtFileSelectionDialog *m_pDataFileDialog;
   SoXtFileSelectionDialog *m_pSettingsFileDialog;
   SoXtFileSelectionDialog *m_pClipmapFileDialog;
   SoXtFileSelectionDialog *m_pVolClippingFileDialog;

   void doClipmapChange(int num, const SbString& username);
   void changeGridDraggerVisibility(SbBool visible);
   void changeVolClipDraggerVisibility(SbBool visible);

   void doClipModelChange(const SbString& username);
  int findComplexityString(const SbString& str);
};


#endif


