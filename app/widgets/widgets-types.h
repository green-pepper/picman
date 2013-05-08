/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __WIDGETS_TYPES_H__
#define __WIDGETS_TYPES_H__


#include "libpicmanwidgets/picmanwidgetstypes.h"

#include "core/core-types.h"

#include "widgets/widgets-enums.h"


/*  input devices & controllers  */

typedef struct _PicmanControllerInfo           PicmanControllerInfo;
typedef struct _PicmanControllerKeyboard       PicmanControllerKeyboard;
typedef struct _PicmanControllerMouse          PicmanControllerMouse;
typedef struct _PicmanControllerWheel          PicmanControllerWheel;
typedef struct _PicmanDeviceInfo               PicmanDeviceInfo;
typedef struct _PicmanDeviceManager            PicmanDeviceManager;


/*  docks  */

typedef struct _PicmanDock                     PicmanDock;
typedef struct _PicmanDockColumns              PicmanDockColumns;
typedef struct _PicmanDockContainer            PicmanDockContainer; /* dummy typedef */
typedef struct _PicmanDockWindow               PicmanDockWindow;
typedef struct _PicmanDockable                 PicmanDockable;
typedef struct _PicmanDockbook                 PicmanDockbook;
typedef struct _PicmanDocked                   PicmanDocked; /* dummy typedef */
typedef struct _PicmanMenuDock                 PicmanMenuDock;
typedef struct _PicmanPanedBox                 PicmanPanedBox;
typedef struct _PicmanToolbox                  PicmanToolbox;


/*  PicmanEditor widgets  */

typedef struct _PicmanColorEditor              PicmanColorEditor;
typedef struct _PicmanDeviceStatus             PicmanDeviceStatus;
typedef struct _PicmanEditor                   PicmanEditor;
typedef struct _PicmanErrorConsole             PicmanErrorConsole;
typedef struct _PicmanToolOptionsEditor        PicmanToolOptionsEditor;


/*  PicmanDataEditor widgets  */

typedef struct _PicmanBrushEditor              PicmanBrushEditor;
typedef struct _PicmanDataEditor               PicmanDataEditor;
typedef struct _PicmanDynamicsEditor           PicmanDynamicsEditor;
typedef struct _PicmanGradientEditor           PicmanGradientEditor;
typedef struct _PicmanPaletteEditor            PicmanPaletteEditor;
typedef struct _PicmanToolPresetEditor         PicmanToolPresetEditor;

/*  PicmanImageEditor widgets  */

typedef struct _PicmanColormapEditor           PicmanColormapEditor;
typedef struct _PicmanComponentEditor          PicmanComponentEditor;
typedef struct _PicmanHistogramEditor          PicmanHistogramEditor;
typedef struct _PicmanImageEditor              PicmanImageEditor;
typedef struct _PicmanSamplePointEditor        PicmanSamplePointEditor;
typedef struct _PicmanSelectionEditor          PicmanSelectionEditor;
typedef struct _PicmanUndoEditor               PicmanUndoEditor;


/*  PicmanContainerView and its implementors  */

typedef struct _PicmanChannelTreeView          PicmanChannelTreeView;
typedef struct _PicmanContainerBox             PicmanContainerBox;
typedef struct _PicmanContainerComboBox        PicmanContainerComboBox;
typedef struct _PicmanContainerEntry           PicmanContainerEntry;
typedef struct _PicmanContainerGridView        PicmanContainerGridView;
typedef struct _PicmanContainerIconView        PicmanContainerIconView;
typedef struct _PicmanContainerTreeStore       PicmanContainerTreeStore;
typedef struct _PicmanContainerTreeView        PicmanContainerTreeView;
typedef struct _PicmanContainerView            PicmanContainerView; /* dummy typedef */
typedef struct _PicmanDrawableTreeView         PicmanDrawableTreeView;
typedef struct _PicmanItemTreeView             PicmanItemTreeView;
typedef struct _PicmanLayerTreeView            PicmanLayerTreeView;
typedef struct _PicmanVectorsTreeView          PicmanVectorsTreeView;

typedef struct _PicmanContainerPopup           PicmanContainerPopup;
typedef struct _PicmanViewableButton           PicmanViewableButton;


/*  PicmanContainerEditor widgets  */

typedef struct _PicmanContainerEditor          PicmanContainerEditor;
typedef struct _PicmanBufferView               PicmanBufferView;
typedef struct _PicmanDocumentView             PicmanDocumentView;
typedef struct _PicmanFontView                 PicmanFontView;
typedef struct _PicmanImageView                PicmanImageView;
typedef struct _PicmanTemplateView             PicmanTemplateView;
typedef struct _PicmanToolEditor               PicmanToolEditor;


/*  PicmanDataFactoryView widgets  */

typedef struct _PicmanBrushFactoryView         PicmanBrushFactoryView;
typedef struct _PicmanDataFactoryView          PicmanDataFactoryView;
typedef struct _PicmanDynamicsFactoryView      PicmanDynamicsFactoryView;
typedef struct _PicmanPatternFactoryView       PicmanPatternFactoryView;
typedef struct _PicmanToolPresetFactoryView    PicmanToolPresetFactoryView;

/*  menus  */

typedef struct _PicmanAction                   PicmanAction;
typedef struct _PicmanActionFactory            PicmanActionFactory;
typedef struct _PicmanActionGroup              PicmanActionGroup;
typedef struct _PicmanEnumAction               PicmanEnumAction;
typedef struct _PicmanMenuFactory              PicmanMenuFactory;
typedef struct _PicmanPlugInAction             PicmanPlugInAction;
typedef struct _PicmanStringAction             PicmanStringAction;
typedef struct _PicmanUIManager                PicmanUIManager;


/*  misc dialogs  */

typedef struct _PicmanColorDialog              PicmanColorDialog;
typedef struct _PicmanErrorDialog              PicmanErrorDialog;
typedef struct _PicmanFileDialog               PicmanFileDialog;
typedef struct _PicmanMessageDialog            PicmanMessageDialog;
typedef struct _PicmanProfileChooserDialog     PicmanProfileChooserDialog;
typedef struct _PicmanProgressDialog           PicmanProgressDialog;
typedef struct _PicmanTextEditor               PicmanTextEditor;
typedef struct _PicmanViewableDialog           PicmanViewableDialog;


/*  PicmanPdbDialog widgets  */

typedef struct _PicmanBrushSelect              PicmanBrushSelect;
typedef struct _PicmanFontSelect               PicmanFontSelect;
typedef struct _PicmanGradientSelect           PicmanGradientSelect;
typedef struct _PicmanPaletteSelect            PicmanPaletteSelect;
typedef struct _PicmanPatternSelect            PicmanPatternSelect;
typedef struct _PicmanPdbDialog                PicmanPdbDialog;


/*  misc widgets  */

typedef struct _PicmanActionEditor             PicmanActionEditor;
typedef struct _PicmanActionView               PicmanActionView;
typedef struct _PicmanBlobEditor               PicmanBlobEditor;
typedef struct _PicmanColorBar                 PicmanColorBar;
typedef struct _PicmanColorDisplayEditor       PicmanColorDisplayEditor;
typedef struct _PicmanColorFrame               PicmanColorFrame;
typedef struct _PicmanColorPanel               PicmanColorPanel;
typedef struct _PicmanComboTagEntry            PicmanComboTagEntry;
typedef struct _PicmanControllerEditor         PicmanControllerEditor;
typedef struct _PicmanControllerList           PicmanControllerList;
typedef struct _PicmanCurveView                PicmanCurveView;
typedef struct _PicmanDashEditor               PicmanDashEditor;
typedef struct _PicmanDeviceEditor             PicmanDeviceEditor;
typedef struct _PicmanDeviceInfoEditor         PicmanDeviceInfoEditor;
typedef struct _PicmanDynamicsOutputEditor     PicmanDynamicsOutputEditor;
typedef struct _PicmanFgBgEditor               PicmanFgBgEditor;
typedef struct _PicmanFgBgView                 PicmanFgBgView;
typedef struct _PicmanFileProcView             PicmanFileProcView;
typedef struct _PicmanFillEditor               PicmanFillEditor;
typedef struct _PicmanGridEditor               PicmanGridEditor;
typedef struct _PicmanHandleBar                PicmanHandleBar;
typedef struct _PicmanHistogramBox             PicmanHistogramBox;
typedef struct _PicmanHistogramView            PicmanHistogramView;
typedef struct _PicmanIconPicker               PicmanIconPicker;
typedef struct _PicmanImageCommentEditor       PicmanImageCommentEditor;
typedef struct _PicmanImageParasiteView        PicmanImageParasiteView;
typedef struct _PicmanImageProfileView         PicmanImageProfileView;
typedef struct _PicmanImagePropView            PicmanImagePropView;
typedef struct _PicmanLanguageComboBox         PicmanLanguageComboBox;
typedef struct _PicmanLanguageEntry            PicmanLanguageEntry;
typedef struct _PicmanLanguageStore            PicmanLanguageStore;
typedef struct _PicmanMessageBox               PicmanMessageBox;
typedef struct _PicmanOverlayBox               PicmanOverlayBox;
typedef struct _PicmanPrefsBox                 PicmanPrefsBox;
typedef struct _PicmanProgressBox              PicmanProgressBox;
typedef struct _PicmanScaleButton              PicmanScaleButton;
typedef struct _PicmanSettingsBox              PicmanSettingsBox;
typedef struct _PicmanSettingsEditor           PicmanSettingsEditor;
typedef struct _PicmanSizeBox                  PicmanSizeBox;
typedef struct _PicmanStrokeEditor             PicmanStrokeEditor;
typedef struct _PicmanTagEntry                 PicmanTagEntry;
typedef struct _PicmanTagPopup                 PicmanTagPopup;
typedef struct _PicmanTemplateEditor           PicmanTemplateEditor;
typedef struct _PicmanTextStyleEditor          PicmanTextStyleEditor;
typedef struct _PicmanThumbBox                 PicmanThumbBox;
typedef struct _PicmanToolPalette              PicmanToolPalette;
typedef struct _PicmanTranslationStore         PicmanTranslationStore;
typedef struct _PicmanWindow                   PicmanWindow;


/*  views  */

typedef struct _PicmanNavigationView           PicmanNavigationView;
typedef struct _PicmanPaletteView              PicmanPaletteView;
typedef struct _PicmanView                     PicmanView;


/*  view renderers  */

typedef struct _PicmanViewRenderer             PicmanViewRenderer;
typedef struct _PicmanViewRendererBrush        PicmanViewRendererBrush;
typedef struct _PicmanViewRendererBuffer       PicmanViewRendererBuffer;
typedef struct _PicmanViewRendererDrawable     PicmanViewRendererDrawable;
typedef struct _PicmanViewRendererGradient     PicmanViewRendererGradient;
typedef struct _PicmanViewRendererImage        PicmanViewRendererImage;
typedef struct _PicmanViewRendererImagefile    PicmanViewRendererImagefile;
typedef struct _PicmanViewRendererLayer        PicmanViewRendererLayer;
typedef struct _PicmanViewRendererPalette      PicmanViewRendererPalette;
typedef struct _PicmanViewRendererVectors      PicmanViewRendererVectors;


/*  cell renderers  */

typedef struct _PicmanCellRendererDashes       PicmanCellRendererDashes;
typedef struct _PicmanCellRendererViewable     PicmanCellRendererViewable;


/*  misc objects  */

typedef struct _PicmanDialogFactory            PicmanDialogFactory;
typedef struct _PicmanTextBuffer               PicmanTextBuffer;
typedef struct _PicmanUIConfigurer             PicmanUIConfigurer;
typedef struct _PicmanWindowStrategy           PicmanWindowStrategy;


/*  session management objects and structs  */

typedef struct _PicmanSessionInfo              PicmanSessionInfo;
typedef struct _PicmanSessionInfoAux           PicmanSessionInfoAux;
typedef struct _PicmanSessionInfoBook          PicmanSessionInfoBook;
typedef struct _PicmanSessionInfoDock          PicmanSessionInfoDock;
typedef struct _PicmanSessionInfoDockable      PicmanSessionInfoDockable;
typedef struct _PicmanSessionManaged           PicmanSessionManaged;


/*  structs  */

typedef struct _PicmanActionEntry              PicmanActionEntry;
typedef struct _PicmanEnumActionEntry          PicmanEnumActionEntry;
typedef struct _PicmanPlugInActionEntry        PicmanPlugInActionEntry;
typedef struct _PicmanRadioActionEntry         PicmanRadioActionEntry;
typedef struct _PicmanStringActionEntry        PicmanStringActionEntry;
typedef struct _PicmanToggleActionEntry        PicmanToggleActionEntry;

typedef struct _PicmanDialogFactoryEntry       PicmanDialogFactoryEntry;


/*  function types  */

typedef GtkWidget * (* PicmanDialogRestoreFunc)        (PicmanDialogFactory *factory,
                                                      GdkScreen         *screen,
                                                      PicmanSessionInfo   *info);
typedef void        (* PicmanActionGroupSetupFunc)     (PicmanActionGroup   *group);
typedef void        (* PicmanActionGroupUpdateFunc)    (PicmanActionGroup   *group,
                                                      gpointer           data);

typedef void        (* PicmanUIManagerSetupFunc)       (PicmanUIManager     *manager,
                                                      const gchar       *ui_path);

typedef void        (* PicmanMenuPositionFunc)         (GtkMenu           *menu,
                                                      gint              *x,
                                                      gint              *y,
                                                      gpointer           data);
typedef gboolean    (* PicmanPanedBoxDroppedFunc)      (GtkWidget         *source,
                                                      gint               insert_index,
                                                      gpointer           data);


#endif /* __WIDGETS_TYPES_H__ */
