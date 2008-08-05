#include <stdlib.h>
#include <stdio.h>

#include <gtk/gtk.h>
#include "SDL.h"

#include "../main/winlnxdefs.h"

extern int GameFreq;
extern Uint32 PrimaryBufferSize;
extern Uint32 SecondaryBufferSize;
extern Uint32 LowBufferLoadLevel;
extern Uint32 HighBufferLoadLevel;
extern Uint8 Resample;
extern int VolPercent;
extern int VolDelta;
extern int VolumeControlType;
extern BOOL SwapChannels;

extern void SaveConfig();

static GtkWidget *defaultFreqLabel, *defaultFreqEntry;
static GtkWidget *priBufferLabel, *priBufferEntry;
static GtkWidget *secBufferLabel, *secBufferEntry;
static GtkWidget *loBufferLabel, *loBufferEntry;
static GtkWidget *hiBufferLabel, *hiBufferEntry;
static GtkWidget *resampleLabel, *resampleCombo;
static GtkWidget *volTypeLabel, *volTypeCombo;
static GtkWidget *volDefaultLabel, *volDefaultSlider;
static GtkWidget *volAdjustLabel, *volAdjustEntry;
static GtkWidget *swapCheck, *swapAlign;

static GtkWidget *displayTable;

void Config_DoConfig();
static void CreateItem(char *text, GtkWidget **label, GtkWidget *configItem, int xPos, int yPos, int colSpan);
static int Config_CreateWindow();
static void okButton_clicked( GtkWidget *widget, void *data );
static void cancelButton_clicked( GtkWidget *widget, void *data );
static char *itoa(int num);


static GtkWidget *configWindow = NULL;

static char *itoa(int num)
{
    static char txt[16];
    sprintf(txt, "%d", num);
    return txt;
}

void Config_DoConfig()
{
    if (!configWindow)
        Config_CreateWindow();

    gtk_widget_show_all( configWindow );
}

static void CreateItem(char *text, GtkWidget **label, GtkWidget *configItem, int xPos, int yPos, int colSpan)
{
    *label = gtk_label_new(text);
    gtk_misc_set_alignment(GTK_MISC(*label), 1, 0.5);
    gtk_table_attach(GTK_TABLE(displayTable), *label, xPos, xPos + 1, yPos, yPos + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 4, 4);
    gtk_table_attach(GTK_TABLE(displayTable), configItem, xPos + 1, xPos + colSpan + 1, yPos, yPos + 1, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 4, 4);
}

static int Config_CreateWindow()
{

    GtkWidget *okButton, *cancelButton;

    // create dialog
    configWindow = gtk_dialog_new();

    gtk_signal_connect_object( GTK_OBJECT(configWindow), "delete-event",
                               GTK_SIGNAL_FUNC(gtk_widget_hide_on_delete), GTK_OBJECT(configWindow) );
    gtk_window_set_title( GTK_WINDOW(configWindow), "JTTL Audio Configuration" );

    displayTable = gtk_table_new( 7, 5, FALSE );
    gtk_container_set_border_width(GTK_CONTAINER(displayTable), 0);
    gtk_container_set_border_width(GTK_CONTAINER(GTK_DIALOG(configWindow)->action_area), 12);
    gtk_table_set_col_spacings( GTK_TABLE(displayTable), 4 );
    gtk_table_set_row_spacings( GTK_TABLE(displayTable), 4 );
    gtk_container_add( GTK_CONTAINER(GTK_DIALOG(configWindow)->vbox), displayTable );

    // ok button
    okButton = gtk_button_new_with_label( "Ok" );
    gtk_signal_connect_object( GTK_OBJECT(okButton), "clicked",
                               GTK_SIGNAL_FUNC(okButton_clicked), NULL );
    gtk_container_add( GTK_CONTAINER(GTK_DIALOG(configWindow)->action_area), okButton );

    // cancel button
    cancelButton = gtk_button_new_with_label( "Cancel" );
    gtk_signal_connect_object( GTK_OBJECT(cancelButton), "clicked",
                               GTK_SIGNAL_FUNC(cancelButton_clicked), NULL );
    gtk_container_add( GTK_CONTAINER(GTK_DIALOG(configWindow)->action_area), cancelButton );


    // Volume Control type
    volTypeCombo = gtk_combo_box_new_text();
    gtk_combo_box_append_text( GTK_COMBO_BOX(volTypeCombo), "Internal SDL" );
    gtk_combo_box_append_text( GTK_COMBO_BOX(volTypeCombo), "OSS mixer" );
    gtk_widget_set_tooltip_text(volTypeCombo, "Internal SDL volume control only affects the volume of Mupen64Plus.  OSS mixer volume control directly controls the OSS mixer, adjusting the master volume for the PC.");
    gtk_widget_set_size_request(volTypeCombo, 120, -1);
    if (VolumeControlType < 1 || VolumeControlType > 2) 
        VolumeControlType = 2;
    gtk_combo_box_set_active(GTK_COMBO_BOX(volTypeCombo), VolumeControlType - 1);
    CreateItem("Volume control:", &volTypeLabel, volTypeCombo, 0, 0, 1);

    // Volume Adjust
    volAdjustEntry = gtk_spin_button_new_with_range(0, 100, 1);
    gtk_spin_button_set_digits(GTK_SPIN_BUTTON(volAdjustEntry), 0);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(volAdjustEntry), VolDelta);
    gtk_entry_set_width_chars(GTK_ENTRY(volAdjustEntry), 4);
    gtk_widget_set_tooltip_text(volAdjustEntry, "Sets the percentage change each time the volume is increased or decreased.");
    CreateItem("Change volume by:", &volAdjustLabel, volAdjustEntry, 2, 0, 1);

    // Resample
    resampleCombo = gtk_combo_box_new_text();
    gtk_combo_box_append_text(GTK_COMBO_BOX(resampleCombo), "Unfiltered");
    gtk_combo_box_append_text(GTK_COMBO_BOX(resampleCombo), "SINC");
    gtk_widget_set_tooltip_text(resampleCombo, "Linear resampling type.\n\n   Unfiltered: less CPU, medium quality\n   SINC: more CPU, higher quality\n\nFor SINC resampling, libsamplerate must be installed.");
    gtk_widget_set_size_request(resampleCombo, 120, -1);
    if (Resample < 1 || Resample > 2) 
        Resample = 1;
    gtk_combo_box_set_active(GTK_COMBO_BOX(resampleCombo), Resample - 1);
    CreateItem("Resampling:", &resampleLabel, resampleCombo, 0, 1, 1);

    // Default Volume
    volDefaultSlider = gtk_hscale_new_with_range(0, 100, 1);
    gtk_scale_set_digits(GTK_SCALE(volDefaultSlider), 0);
    gtk_scale_set_draw_value(GTK_SCALE(volDefaultSlider), TRUE);
    gtk_scale_set_value_pos(GTK_SCALE(volDefaultSlider), GTK_POS_TOP);
    gtk_range_set_value(GTK_RANGE(volDefaultSlider), VolPercent);
    gtk_widget_set_tooltip_text(volDefaultSlider, "Sets the default volume upon startup when using Internal SDL volume control.  If Internal SDL is not used, this value is unused and the default volume is the volume that the harware mixer is set to when Mupen64Plus loads.");
    CreateItem("Default volume:", &volDefaultLabel, volDefaultSlider, 0, 2, 3);

    // Swap Channels
    swapCheck = gtk_check_button_new_with_label("Swap L/R channels");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(swapCheck), SwapChannels);
    swapAlign = gtk_hbox_new(0, 0);
    gtk_box_pack_start(GTK_BOX(swapAlign), swapCheck, FALSE, FALSE, 0);
    gtk_table_attach(GTK_TABLE(displayTable), swapAlign, 1, 2, 3, 4, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 4, 4);

    // Default frequency
    defaultFreqEntry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(defaultFreqEntry), itoa(GameFreq));
    gtk_entry_set_width_chars(GTK_ENTRY(defaultFreqEntry), 8);
    gtk_widget_set_tooltip_text(defaultFreqEntry, "This sets default frequency used if rom doesn't want to change it.  Probably the only game that needs this is Zelda: Ocarina Of Time Master Quest.");
    CreateItem("Default frequency:", &defaultFreqLabel, defaultFreqEntry, 2, 3, 1);

    // Primary buffer
    priBufferEntry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(priBufferEntry), itoa(PrimaryBufferSize));
    gtk_entry_set_width_chars(GTK_ENTRY(priBufferEntry), 8);
    gtk_widget_set_tooltip_text(priBufferEntry, "Size of primary buffer in bytes. This is the buffer where audio is loaded after it's extracted from the N64's memory.");
    CreateItem("Primary buffer:", &priBufferLabel, priBufferEntry, 0, 4, 1);

    // Secondary buffer
    secBufferEntry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(secBufferEntry), itoa(SecondaryBufferSize));
    gtk_entry_set_width_chars(GTK_ENTRY(secBufferEntry), 8);
    gtk_widget_set_tooltip_text(secBufferEntry, "This is SDL's hardware buffer size. This is the number of samples, so the actual buffer size is four times this number.");
    CreateItem("Secondary buffer:", &secBufferLabel, secBufferEntry, 0, 5, 1);

    // Buffer low
    loBufferEntry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(loBufferEntry), itoa(LowBufferLoadLevel));
    gtk_entry_set_width_chars(GTK_ENTRY(loBufferEntry), 8);
    gtk_widget_set_tooltip_text(loBufferEntry, "If buffer load goes under this level, then the game will speed up to fill the buffer.  You probably want to adjust this value if you get dropouts.");
    CreateItem("Buffer low level:", &loBufferLabel, loBufferEntry, 2, 4, 1);

    // Buffer high
    hiBufferEntry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(hiBufferEntry), itoa(HighBufferLoadLevel));
    gtk_entry_set_width_chars(GTK_ENTRY(hiBufferEntry), 8);
    gtk_widget_set_tooltip_text(hiBufferEntry, "If buffer load exceeds this level, then some extra slowdown is added to prevent buffer overflow.");
    CreateItem("Buffer high level:", &hiBufferLabel, hiBufferEntry, 2, 5, 1);

    return 0;
}

static void okButton_clicked( GtkWidget *widget, void *data )
{
    GameFreq = atoi(gtk_entry_get_text(GTK_ENTRY(defaultFreqEntry)));
    PrimaryBufferSize = atoi(gtk_entry_get_text(GTK_ENTRY(priBufferEntry)));
    SecondaryBufferSize = atoi(gtk_entry_get_text(GTK_ENTRY(secBufferEntry)));
    LowBufferLoadLevel = atoi(gtk_entry_get_text(GTK_ENTRY(loBufferEntry)));
    HighBufferLoadLevel = atoi(gtk_entry_get_text(GTK_ENTRY(hiBufferEntry)));
    Resample = gtk_combo_box_get_active(GTK_COMBO_BOX(resampleCombo)) + 1;
    VolPercent = gtk_range_get_value(GTK_RANGE(volDefaultSlider));
    VolDelta = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(volAdjustEntry));
    VolumeControlType = gtk_combo_box_get_active(GTK_COMBO_BOX(volTypeCombo)) + 1;
    SwapChannels = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(swapCheck));
    SaveConfig();
    gtk_widget_destroy(configWindow);
    configWindow = NULL;
}

static void cancelButton_clicked( GtkWidget *widget, void *data )
{
    gtk_widget_destroy(configWindow);
    configWindow = NULL;
}
