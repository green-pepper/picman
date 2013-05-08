/*  picmanrc-blurbs.h  --  descriptions for picmanrc properties  */

#ifndef __PICMAN_RC_BLURBS_H__
#define __PICMAN_RC_BLURBS_H__


/*  Not all strings defined here are used in the user interface
 *  (the preferences dialog mainly) and only those that are should
 *  be marked for translation.
 */

#define ACTIVATE_ON_FOCUS_BLURB \
N_("When enabled, an image will become the active image when its image " \
   "window receives the focus. This is useful for window managers using " \
   "\"click to focus\".")

#define BRUSH_PATH_BLURB \
"Sets the brush search path."

#define BRUSH_PATH_WRITABLE_BLURB ""

#define DYNAMICS_PATH_BLURB \
N_("Sets the dynamics search path.")

#define DYNAMICS_PATH_WRITABLE_BLURB ""

#define TOOL_PRESET_PATH_BLURB \
N_("Sets the dynamics search path.")

#define TOOL_PRESET_PATH_WRITABLE_BLURB ""

#define CANVAS_PADDING_COLOR_BLURB \
N_("Sets the canvas padding color used if the padding mode is set to " \
   "custom color.")

#define CANVAS_PADDING_MODE_BLURB \
N_("Specifies how the area around the image should be drawn.")

#define COLOR_MANAGEMENT_BLURB \
"Defines the color management behavior."

#define COLOR_PROFILE_POLICY_BLURB \
N_("How to handle embedded color profiles when opening a file.")

#define CURSOR_FORMAT_BLURB \
N_("Sets the pixel format to use for mouse pointers.")

#define CURSOR_MODE_BLURB \
N_("Sets the type of mouse pointers to use.")

#define CURSOR_HANDEDNESS_BLURB \
N_("Sets the handedness for cursor positioning.")

#define CURSOR_UPDATING_BLURB \
N_("Context-dependent mouse pointers are helpful.  They are enabled by " \
   "default.  However, they require overhead that you may want to do without.")

#define DEFAULT_BRUSH_BLURB \
"Specify a default brush.  The brush is searched for in the " \
"specified brush path."

#define DEFAULT_DYNAMICS_BLURB \
"Specify a default dynamics.  The dynamics is searched for in the " \
"specified dynamics path."

#define DEFAULT_TOOL_PRESET_BLURB \
"Specify a default tool preset.  The tool preset is searched for in the " \
"specified tool prests path."

#define DEFAULT_DOT_FOR_DOT_BLURB \
N_("When enabled, this will ensure that each pixel of an image gets " \
   "mapped to a pixel on the screen.")

#define DEFAULT_FONT_BLURB \
"Specify a default font."

#define DEFAULT_GRADIENT_BLURB \
"Specify a default gradient."

#define DEFAULT_GRID_BLURB \
"Specify a default image grid."

#define DEFAULT_IMAGE_BLURB \
"Sets the default image in the \"File/New\" dialog."

#define DEFAULT_PATTERN_BLURB \
"Specify a default pattern."

#define DEFAULT_PALETTE_BLURB \
"Specify a default palette."

#define DEFAULT_SNAP_DISTANCE_BLURB \
N_("This is the distance in pixels where Guide and Grid snapping " \
   "activates.")

#define DEFAULT_SNAP_TO_GUIDES_BLURB \
"Snap to guides by default in new image windows."

#define DEFAULT_SNAP_TO_GRID_BLURB \
"Snap to the grid by default in new image windows."

#define DEFAULT_SNAP_TO_CANVAS_BLURB \
"Snap to the canvas edge by default in new image windows."

#define DEFAULT_SNAP_TO_PATH_BLURB \
"Snap to the active path by default in new image windows."

#define DEFAULT_THRESHOLD_BLURB \
N_("Tools such as fuzzy-select and bucket fill find regions based on a " \
   "seed-fill algorithm.  The seed fill starts at the initially selected " \
   "pixel and progresses in all directions until the difference of pixel " \
   "intensity from the original is greater than a specified threshold. " \
   "This value represents the default threshold.")

#define DEFAULT_VIEW_BLURB \
"Sets the default settings for the image view."

#define DEFAULT_FULLSCREEN_VIEW_BLURB \
"Sets the default settings used when an image is viewed in fullscreen mode."

#define DOCK_WINDOW_HINT_BLURB \
N_("The window type hint that is set on dock windows and the toolbox " \
   "window. This may affect the way your window manager decorates and " \
   "handles these windows.")

#define ENVIRON_PATH_BLURB \
"Sets the environ search path."

#define FRACTALEXPLORER_PATH_BLURB \
"Where to search for fractals used by the Fractal Explorer plug-in."

#define GAMMA_CORRECTION_BLURB \
"This setting is ignored."
#if 0
"Sets the gamma correction value for the display. 1.0 corresponds to no " \
"gamma correction.  For most displays, gamma correction should be set " \
"to between 2.0 and 2.6. One important thing to keep in mind: Many images " \
"that you might get from outside sources will in all likelihood already " \
"be gamma-corrected.  In these cases, the image will look washed-out if " \
"PICMAN has gamma-correction turned on.  If you are going to work with " \
"images of this sort, turn gamma correction off by setting the value to 1.0."
#endif

#define GFIG_PATH_BLURB \
"Where to search for Gfig figures used by the Gfig plug-in."

#define GFLARE_PATH_BLURB \
"Where to search for gflares used by the GFlare plug-in."

#define PICMANRESSIONIST_PATH_BLURB \
"Where to search for data used by the Picmanressionist plug-in."

#define GLOBAL_BRUSH_BLURB \
N_("When enabled, the selected brush will be used for all tools.")

#define GLOBAL_DYNAMICS_BLURB \
N_("When enabled, the selected dynamics will be used for all tools.")

#define GLOBAL_FONT_BLURB \
"When enabled, the selected font will be used for all tools."

#define GLOBAL_GRADIENT_BLURB \
N_("When enabled, the selected gradient will be used for all tools.")

#define GLOBAL_PATTERN_BLURB \
N_("When enabled, the selected pattern will be used for all tools.")

#define GLOBAL_PALETTE_BLURB \
"When enabled, the selected palette will be used for all tools."

#define GRADIENT_PATH_BLURB \
"Sets the gradient search path."

#define GRADIENT_PATH_WRITABLE_BLURB ""

#define FONT_PATH_BLURB \
"Where to look for fonts in addition to the system-wide installed fonts."

#define HELP_BROWSER_BLURB \
N_("Sets the browser used by the help system.")

#define HELP_LOCALES_BLURB \
"Specifies the language preferences used by the help system. This is a " \
"colon-separated list of language identifiers with decreasing priority. " \
"If empty, the language is taken from the user's locale setting."

#define IMAGE_MAP_TOOL_MAX_RECENT_BLURB \
"How many recent settings to keep around in color correction tools"

#define IMAGE_STATUS_FORMAT_BLURB \
N_("Sets the text to appear in image window status bars.")

#define IMAGE_TITLE_FORMAT_BLURB \
N_("Sets the text to appear in image window titles.")

#define INITIAL_ZOOM_TO_FIT_BLURB \
N_("When enabled, this will ensure that the full image is visible after a " \
   "file is opened, otherwise it will be displayed with a scale of 1:1.")

#define INTERPOLATION_TYPE_BLURB \
N_("Sets the level of interpolation used for scaling and other " \
   "transformations.")

#define INTERPRETER_PATH_BLURB \
"Sets the interpreter search path."

#define LANGUAGE_BLURB \
N_("Specifies the language to use for the user interface.")

#define LAST_OPENED_SIZE_BLURB \
N_("How many recently opened image filenames to keep on the File menu.")

#define MARCHING_ANTS_SPEED_BLURB \
N_("Speed of marching ants in the selection outline.  This value is in " \
   "milliseconds (less time indicates faster marching).")

#define MAX_NEW_IMAGE_SIZE_BLURB  \
N_("PICMAN will warn the user if an attempt is made to create an image that " \
   "would take more memory than the size specified here.")

#define MODULE_PATH_BLURB \
"Sets the module search path."

#define MONITOR_RES_FROM_GDK_BLURB \
"When enabled, PICMAN will use the monitor resolution from the windowing system."

#define MONITOR_XRESOLUTION_BLURB \
N_("Sets the monitor's horizontal resolution, in dots per inch.  If set to " \
   "0, forces the X server to be queried for both horizontal and vertical " \
   "resolution information.")

#define MONITOR_YRESOLUTION_BLURB \
N_("Sets the monitor's vertical resolution, in dots per inch.  If set to " \
   "0, forces the X server to be queried for both horizontal and vertical " \
   "resolution information.")

#define MOVE_TOOL_CHANGES_ACTIVE_BLURB \
N_("If enabled, the move tool sets the edited layer or path as active.  " \
   "This used to be the default behaviour in older versions.")

#define NAVIGATION_PREVIEW_SIZE_BLURB \
N_("Sets the size of the navigation preview available in the lower right " \
   "corner of the image window.")

#define NUM_PROCESSORS_BLURB \
N_("Sets how many processors PICMAN should try to use simultaneously.")

#define PALETTE_PATH_BLURB \
"Sets the palette search path."

#define PALETTE_PATH_WRITABLE_BLURB ""

#define PATTERN_PATH_BLURB \
"Sets the pattern search path."

#define PATTERN_PATH_WRITABLE_BLURB ""

#define PERFECT_MOUSE_BLURB \
N_("When enabled, the X server is queried for the mouse's current position " \
   "on each motion event, rather than relying on the position hint.  This " \
   "means painting with large brushes should be more accurate, but it may " \
   "be slower.  Conversely, on some X servers enabling this option results " \
   "in faster painting.")

#define PLUG_IN_HISTORY_SIZE_BLURB \
"How many recently used plug-ins to keep on the Filters menu."

#define PLUG_IN_PATH_BLURB \
"Sets the plug-in search path."

#define PLUGINRC_PATH_BLURB \
"Sets the pluginrc search path."

#define LAYER_PREVIEWS_BLURB \
N_("Sets whether PICMAN should create previews of layers and channels. " \
   "Previews in the layers and channels dialog are nice to have but they " \
   "can slow things down when working with large images.")

#define LAYER_PREVIEW_SIZE_BLURB \
N_("Sets the preview size used for layers and channel previews in newly " \
   "created dialogs.")

#define QUICK_MASK_COLOR_BLURB \
N_("Sets the default quick mask color.")

#define RESIZE_WINDOWS_ON_RESIZE_BLURB \
N_("When enabled, the image window will automatically resize itself " \
   "whenever the physical image size changes.")

#define RESIZE_WINDOWS_ON_ZOOM_BLURB \
N_("When enabled, the image window will automatically resize itself " \
   "when zooming into and out of images.")

#define RESTORE_SESSION_BLURB \
N_("Let PICMAN try to restore your last saved session on each startup.")

#define SAVE_DEVICE_STATUS_BLURB \
N_("Remember the current tool, pattern, color, and brush across PICMAN " \
   "sessions.")

#define SAVE_DOCUMENT_HISTORY_BLURB \
N_("Keep a permanent record of all opened and saved files in the Recent " \
   "Documents list.")

#define SAVE_SESSION_INFO_BLURB \
N_("Save the positions and sizes of the main dialogs when PICMAN exits.")

#define SAVE_TOOL_OPTIONS_BLURB \
N_("Save the tool options when PICMAN exits.")

#define SCRIPT_FU_PATH_BLURB \
"This path will be searched for scripts when the Script-Fu plug-in is run."

#define SHOW_BRUSH_OUTLINE_BLURB \
N_("When enabled, all brush-based paint tools will show a preview of the current " \
   "brush's outline.")

#define SHOW_HELP_BUTTON_BLURB \
N_("When enabled, dialogs will show a help button that gives access to " \
   "the related help page.  Without this button, the help page can still " \
   "be reached by pressing F1.")

#define SHOW_PAINT_TOOL_CURSOR_BLURB \
N_("When enabled, the mouse pointer will be shown over the image while " \
    "using a brush-based paint tool.")

#define SHOW_MENUBAR_BLURB \
N_("When enabled, the menubar is visible by default. This can also be " \
   "toggled with the \"View->Show Menubar\" command.")

#define SHOW_RULERS_BLURB \
N_("When enabled, the rulers are visible by default. This can also be " \
   "toggled with the \"View->Show Rulers\" command.")

#define SHOW_SCROLLBARS_BLURB \
N_("When enabled, the scrollbars are visible by default. This can also be " \
   "toggled with the \"View->Show Scrollbars\" command.")

#define SHOW_STATUSBAR_BLURB \
N_("When enabled, the statusbar is visible by default. This can also be " \
   "toggled with the \"View->Show Statusbar\" command.")

#define SHOW_SELECTION_BLURB \
N_("When enabled, the selection is visible by default. This can also be " \
   "toggled with the \"View->Show Selection\" command.")

#define SHOW_LAYER_BOUNDARY_BLURB \
N_("When enabled, the layer boundary is visible by default. This can also " \
   "be toggled with the \"View->Show Layer Boundary\" command.")

#define SHOW_GUIDES_BLURB \
N_("When enabled, the guides are visible by default. This can also be " \
   "toggled with the \"View->Show Guides\" command.")

#define SHOW_GRID_BLURB \
N_("When enabled, the grid is visible by default. This can also be toggled " \
   "with the \"View->Show Grid\" command.")

#define SHOW_SAMPLE_POINTS_BLURB \
N_("When enabled, the sample points are visible by default. This can also be " \
   "toggled with the \"View->Show Sample Points\" command.")

#define SHOW_TOOLTIPS_BLURB \
N_("Show a tooltip when the pointer hovers over an item.")

#define SINGLE_WINDOW_MODE_BLURB \
N_("Use PICMAN in a single-window mode.")

#define HIDE_DOCKS_BLURB \
N_("Hide docks and other windows, leaving only image windows.")

#define SPACE_BAR_ACTION_BLURB \
N_("What to do when the space bar is pressed in the image window.")

#define SWAP_PATH_BLURB \
N_("Sets the swap file location. PICMAN uses a tile based memory allocation " \
   "scheme. The swap file is used to quickly and easily swap tiles out to " \
   "disk and back in. Be aware that the swap file can easily get very large " \
   "if PICMAN is used with large images. " \
   "Also, things can get horribly slow if the swap file is created on " \
   "a folder that is mounted over NFS.  For these reasons, it may be " \
   "desirable to put your swap file in \"/tmp\".")

#define TEAROFF_MENUS_BLURB \
N_("When enabled, menus can be torn off.")

#define CAN_CHANGE_ACCELS_BLURB \
N_("When enabled, you can change keyboard shortcuts for menu items " \
   "by hitting a key combination while the menu item is highlighted.")

#define SAVE_ACCELS_BLURB \
N_("Save changed keyboard shortcuts when PICMAN exits.")

#define RESTORE_ACCELS_BLURB \
N_("Restore saved keyboard shortcuts on each PICMAN startup.")

#define TEMP_PATH_BLURB \
N_("Sets the folder for temporary storage. Files will appear here " \
   "during the course of running PICMAN.  Most files will disappear " \
   "when PICMAN exits, but some files are likely to remain, so it " \
   "is best if this folder not be one that is shared by other users.")

#define THEME_BLURB \
"The name of the theme to use."

#define THEME_PATH_BLURB \
"Sets the theme search path."

#define THUMBNAIL_SIZE_BLURB \
N_("Sets the size of the thumbnail shown in the Open dialog.")

#define THUMBNAIL_FILESIZE_LIMIT_BLURB \
N_("The thumbnail in the Open dialog will be automatically updated " \
   "if the file being previewed is smaller than the size set here.")

#define TILE_CACHE_SIZE_BLURB \
N_("When the amount of pixel data exceeds this limit, PICMAN will start to " \
   "swap tiles to disk.  This is a lot slower but it makes it possible to " \
   "work on images that wouldn't fit into memory otherwise.  If you have a " \
   "lot of RAM, you may want to set this to a higher value.")

#define TOOLBOX_COLOR_AREA_BLURB \
N_("Show the current foreground and background colors in the toolbox.")

#define TOOLBOX_FOO_AREA_BLURB \
N_("Show the currently selected brush, pattern and gradient in the toolbox.")

#define TOOLBOX_IMAGE_AREA_BLURB \
N_("Show the currently active image in the toolbox.")

#define TOOLBOX_WILBER_BLURB \
"Show the PICMAN mascot at the top of the toolbox."

#define TRANSPARENCY_TYPE_BLURB \
N_("Sets the manner in which transparency is displayed in images.")

#define TRANSPARENCY_SIZE_BLURB \
N_("Sets the size of the checkerboard used to display transparency.")

#define TRUST_DIRTY_FLAG_BLURB \
N_("When enabled, PICMAN will not save an image if it has not been changed " \
   "since it was opened.")

#define UNDO_LEVELS_BLURB \
N_("Sets the minimal number of operations that can be undone. More undo " \
   "levels are kept available until the undo-size limit is reached.")

#define UNDO_SIZE_BLURB \
N_("Sets an upper limit to the memory that is used per image to keep " \
   "operations on the undo stack. Regardless of this setting, at least " \
   "as many undo-levels as configured can be undone.")

#define UNDO_PREVIEW_SIZE_BLURB \
N_("Sets the size of the previews in the Undo History.")

#define USE_HELP_BLURB  \
N_("When enabled, pressing F1 will open the help browser.")

#define USER_MANUAL_ONLINE_BLURB  \
"When enabled, the online user manual will be used by the help system. " \
"Otherwise the locally installed copy is used."

#define USER_MANUAL_ONLINE_URI_BLURB \
"The location of the online user manual. This is used if " \
"'user-manual-online' is enabled."

#define ZOOM_QUALITY_BLURB \
"There's a tradeoff between speed and quality of the zoomed-out display."

#define DEFAULT_USE_EVENT_HISTORY_BLURB \
"Bugs in event history buffer are frequent so in case of cursor " \
"offset problems turning it off helps."

#endif  /* __PICMAN_RC_BLURBS_H__ */
