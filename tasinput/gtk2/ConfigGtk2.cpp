//
// TASinput Configuration Screen for GTK+2.xx
// written by NMN
// --
// I may have took some 'tips' from Glade. Sue me.
//

#include "Config.h"
#include "../main/translate.h"

GtkWidget* create_ControllerEdit (void)
{
    GtkWidget *ControllerEdit;
    GtkWidget *ControllerEditFixedContainer;
    GtkWidget *ControllerEditDeviceListFrame;
    GtkWidget *ControllerEditDeviceListFrameAlignment;
    GtkWidget *ControllerEditDeviceListDeviceWindow;
    GtkWidget *ControllerEditDeviceListDeviceView;
    GtkWidget *ControllerEditDeviceListFrameLabel;
    GtkWidget *ControllerEditDirectionalContainer;
    GtkWidget *ControllerEditAnalogStickFrame;
    GtkWidget *ControllerEditAnaologStickFrameAlignment;
    GtkWidget *ControllerEditAnalogStickFrameTable;
    GtkWidget *ControllerEditAnalogStickAssignU;
    GtkWidget *ControllerEditAnalogStickAssignD;
    GtkWidget *ControllerEditAnalogStickAssignL;
    GtkWidget *ControllerEditAnalogStickAssignR;
    GtkWidget *ControllerEditAnalogStickContentsU;
    GtkWidget *ControllerEditAnalogStickContentsD;
    GtkWidget *ControllerEditAnalogStickContentsL;
    GtkWidget *ControllerEditAnalogStickContentsR;
    GtkWidget *ControllerEditAnalogStickFrameLabel;
    GtkWidget *ControllerEditDPadFrame;
    GtkWidget *ControllerEditDPadFrameAlignment;
    GtkWidget *ControllerEditDPadFrameTable;
    GtkWidget *ControllerEditDPadAssignU;
    GtkWidget *ControllerEditDPadAssignD;
    GtkWidget *ControllerEditDPadAssignL;
    GtkWidget *ControllerEditDPadAssignR;
    GtkWidget *ControllerEditDPadContentsU;
    GtkWidget *ControllerEditDPadContentsD;
    GtkWidget *ControllerEditDPadContentsL;
    GtkWidget *ControllerEditDPadContentsR;
    GtkWidget *ControllerEditDPadFrameLabel;
    GtkWidget *ControllerEditCButtonsFrame;
    GtkWidget *ControllerEditCButtonsFrameAlignment;
    GtkWidget *ControllerEditCButtonsFrameTable;
    GtkWidget *ControllerEditCButtonsAssignU;
    GtkWidget *ControllerEditCButtonsAssignD;
    GtkWidget *ControllerEditCButtonsAssignL;
    GtkWidget *ControllerEditCButtonsAssignR;
    GtkWidget *ControllerEditCButtonsContentsU;
    GtkWidget *ControllerEditCButtonsContentsD;
    GtkWidget *ControllerEditCButtonsContentsL;
    GtkWidget *ControllerEditCButtonsContentsR;
    GtkWidget *ControllerEditCButtonsFrameLabel;
    GtkWidget *ControllerEditSpeedModifiersFrame;
    GtkWidget *ControllerEditSpeedModifiersFrameAlignment;
    GtkWidget *ControllerEditSpeedModifiersFrameTable;
    GtkWidget *ControllerEditSpeedModifiersAssignM1;
    GtkWidget *ControllerEditSpeedModifiersAssignM2;
    GtkWidget *ControllerEditSpeedModifiersContentsM1;
    GtkWidget *ControllerEditSpeedModifiersContentsM2;
    GtkObject *ControllerEditSpeedModifiersSpinButtonM1_adj;
    GtkWidget *ControllerEditSpeedModifiersSpinButtonM1;
    GtkObject *ControllerEditSpeedModifiersSpinButtonM2_adj;
    GtkWidget *ControllerEditSpeedModifiersSpinButtonM2;
    GtkWidget *ControllerEditSpeedModifiersFrameLabel;
    GtkWidget *ControllerEditMacrosFrame;
    GtkWidget *ControllerEditMacrosFrameAlignment;
    GtkWidget *ControllerEditMacrosFrameTable;
    GtkWidget *ControllerEditMacrosAssignC1;
    GtkWidget *ControllerEditMacrosAssignC2;
    GtkWidget *ControllerEditMacrosAssignC3;
    GtkWidget *ControllerEditMacrosAssignC4;
    GtkWidget *ControllerEditMacrosAssignC5;
    GtkWidget *ControllerEditMacrosContentsC5;
    GtkWidget *ControllerEditMacrosContentsC4;
    GtkWidget *ControllerEditMacrosContentsC3;
    GtkWidget *ControllerEditMacrosContentsC2;
    GtkWidget *ControllerEditMacrosContentsC1;
    GtkWidget *ControllerEditMacrosEditC1;
    GtkWidget *ControllerEditMacrosEditC2;
    GtkWidget *ControllerEditMacrosEditC3;
    GtkWidget *ControllerEditMacrosEditC4;
    GtkWidget *ControllerEditMacrosEditC5;
    GtkWidget *ControllerEditMacrosFrameLabel;
    GtkWidget *ControllerEditButtonStrip;
    GtkWidget *ControllerEditReset;
    GtkWidget *ControllerEditLoadDef;
    GtkWidget *ControllerEditSaveDef;
    GtkWidget *ControllerEditApply;
    GtkWidget *ControllerEditOK;
    GtkWidget *ControllerEditCancel;
    GtkWidget *ControllerEditCurrentControllerLabel;
    GtkWidget *ControllerEditButtonsFrame;
    GtkWidget *ControllerEditButtonsFrameAlignment;
    GtkWidget *ControllerEditButtonsFrameTable;
    GtkWidget *ControllerEditButtonsAssignS;
    GtkWidget *ControllerEditButtonsAssignB;
    GtkWidget *ControllerEditButtonsAssignA;
    GtkWidget *ControllerEditButtonsContentsA;
    GtkWidget *ControllerEditButtonsContentsB;
    GtkWidget *ControllerEditButtonsContentsS;
    GtkWidget *ControllerEditButtonsFrameLabel;
    GtkWidget *ControllerEditTriggersFrame;
    GtkWidget *ControllerEditTriggersFrameAlignment;
    GtkWidget *ControllerEditTriggersFrameTable;
    GtkWidget *ControllerEditTriggersAssignZ;
    GtkWidget *ControllerEditTriggersAssignL;
    GtkWidget *ControllerEditTriggersAssignR;
    GtkWidget *ControllerEditTriggersContentsZ;
    GtkWidget *ControlllerEditTriggersContentsL;
    GtkWidget *ControllerEditTriggersContentsR;
    GtkWidget *ControllerEditTriggersFrameLabel;
    GtkWidget *ControllerEditCurrentControllerCombobox;
    GtkWidget *ControllerEditSensitivityFrame;
    GtkWidget *ControllerEditSensitivityFrameAlignment;
    GtkWidget *ControllerEditSensitivityFrameTable;
    GtkWidget *ControllerEditSensitivityMinLabel;
    GtkWidget *ControllerEditSensitivityMaxLabel;
    GtkObject *ControllerEditSensitivityMaxSpinbox_adj;
    GtkWidget *ControllerEditSensitivityMaxSpinbox;
    GtkObject *ControllerEditSensitivityMinSpinbox_adj;
    GtkWidget *ControllerEditSensitivityMinSpinbox;
    GtkWidget *ControllerEditSensitivityFrameLabel;
    GtkWidget *ControllerEditPropertiesFrame;
    GtkWidget *ControllerEditPropertiesFrameAlignment;
    GtkWidget *ControllerEditProperties;
    GtkWidget *ControllerEditPropertiesActiveCheckbox;
    GtkWidget *ControllerEditPropertiesMemPakCheckbox;
    GtkWidget *ControllerEditPropertiesFrameLabel;

    ControllerEdit = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (ControllerEdit), tr("Controller"));

    ControllerEditFixedContainer = gtk_fixed_new ();
    gtk_widget_show (ControllerEditFixedContainer);
    gtk_container_add (GTK_CONTAINER (ControllerEdit), ControllerEditFixedContainer);

    ControllerEditDeviceListFrame = gtk_frame_new (NULL);
    gtk_widget_show (ControllerEditDeviceListFrame);
    gtk_fixed_put (GTK_FIXED (ControllerEditFixedContainer), ControllerEditDeviceListFrame, 232, 16);
    gtk_widget_set_size_request (ControllerEditDeviceListFrame, 313, 87);

    ControllerEditDeviceListFrameAlignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (ControllerEditDeviceListFrameAlignment);
    gtk_container_add (GTK_CONTAINER (ControllerEditDeviceListFrame), ControllerEditDeviceListFrameAlignment);
    gtk_alignment_set_padding (GTK_ALIGNMENT (ControllerEditDeviceListFrameAlignment), 0, 6, 6, 6);

    ControllerEditDeviceListDeviceWindow = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_show (ControllerEditDeviceListDeviceWindow);
    gtk_container_add (GTK_CONTAINER (ControllerEditDeviceListFrameAlignment), ControllerEditDeviceListDeviceWindow);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (ControllerEditDeviceListDeviceWindow), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (ControllerEditDeviceListDeviceWindow), GTK_SHADOW_IN);

    ControllerEditDeviceListDeviceView = gtk_tree_view_new ();
    gtk_widget_show (ControllerEditDeviceListDeviceView);
    gtk_container_add (GTK_CONTAINER (ControllerEditDeviceListDeviceWindow), ControllerEditDeviceListDeviceView);

    ControllerEditDeviceListFrameLabel = gtk_label_new (tr("Device List"));
    gtk_widget_show (ControllerEditDeviceListFrameLabel);
    gtk_frame_set_label_widget (GTK_FRAME (ControllerEditDeviceListFrame), ControllerEditDeviceListFrameLabel);
    gtk_label_set_use_markup (GTK_LABEL (ControllerEditDeviceListFrameLabel), TRUE);

    ControllerEditDirectionalContainer = gtk_vbox_new (TRUE, 5);
    gtk_widget_show (ControllerEditDirectionalContainer);
    gtk_fixed_put (GTK_FIXED (ControllerEditFixedContainer), ControllerEditDirectionalContainer, 120, 112);
    gtk_widget_set_size_request (ControllerEditDirectionalContainer, 239, 236);

    ControllerEditAnalogStickFrame = gtk_frame_new (NULL);
    gtk_widget_show (ControllerEditAnalogStickFrame);
    gtk_box_pack_start (GTK_BOX (ControllerEditDirectionalContainer), ControllerEditAnalogStickFrame, TRUE, TRUE, 0);

    ControllerEditAnaologStickFrameAlignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (ControllerEditAnaologStickFrameAlignment);
    gtk_container_add (GTK_CONTAINER (ControllerEditAnalogStickFrame), ControllerEditAnaologStickFrameAlignment);
    gtk_alignment_set_padding (GTK_ALIGNMENT (ControllerEditAnaologStickFrameAlignment), 0, 6, 6, 6);

    ControllerEditAnalogStickFrameTable = gtk_table_new (2, 4, FALSE);
    gtk_widget_show (ControllerEditAnalogStickFrameTable);
    gtk_container_add (GTK_CONTAINER (ControllerEditAnaologStickFrameAlignment), ControllerEditAnalogStickFrameTable);

    ControllerEditAnalogStickAssignU = gtk_button_new_with_mnemonic (tr("U"));
    gtk_widget_show (ControllerEditAnalogStickAssignU);
    gtk_table_attach (GTK_TABLE (ControllerEditAnalogStickFrameTable), ControllerEditAnalogStickAssignU, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                       (GtkAttachOptions) (0), 0, 0);

    ControllerEditAnalogStickAssignD = gtk_button_new_with_mnemonic (tr("D"));
    gtk_widget_show (ControllerEditAnalogStickAssignD);
    gtk_table_attach (GTK_TABLE (ControllerEditAnalogStickFrameTable), ControllerEditAnalogStickAssignD, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                       (GtkAttachOptions) (0), 0, 0);

    ControllerEditAnalogStickAssignL = gtk_button_new_with_mnemonic (tr("L"));
    gtk_widget_show (ControllerEditAnalogStickAssignL);
    gtk_table_attach (GTK_TABLE (ControllerEditAnalogStickFrameTable), ControllerEditAnalogStickAssignL, 2, 3, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                       (GtkAttachOptions) (0), 0, 0);

    ControllerEditAnalogStickAssignR = gtk_button_new_with_mnemonic (tr("R"));
    gtk_widget_show (ControllerEditAnalogStickAssignR);
    gtk_table_attach (GTK_TABLE (ControllerEditAnalogStickFrameTable), ControllerEditAnalogStickAssignR, 2, 3, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                       (GtkAttachOptions) (0), 0, 0);

    ControllerEditAnalogStickContentsU = gtk_entry_new ();
    gtk_widget_show (ControllerEditAnalogStickContentsU);
    gtk_table_attach (GTK_TABLE (ControllerEditAnalogStickFrameTable), ControllerEditAnalogStickContentsU, 1, 2, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);
    gtk_editable_set_editable (GTK_EDITABLE (ControllerEditAnalogStickContentsU), FALSE);
    gtk_entry_set_invisible_char (GTK_ENTRY (ControllerEditAnalogStickContentsU), 9679);

    ControllerEditAnalogStickContentsD = gtk_entry_new ();
    gtk_widget_show (ControllerEditAnalogStickContentsD);
    gtk_table_attach (GTK_TABLE (ControllerEditAnalogStickFrameTable), ControllerEditAnalogStickContentsD, 1, 2, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);
    gtk_editable_set_editable (GTK_EDITABLE (ControllerEditAnalogStickContentsD), FALSE);
    gtk_entry_set_invisible_char (GTK_ENTRY (ControllerEditAnalogStickContentsD), 9679);

    ControllerEditAnalogStickContentsL = gtk_entry_new ();
    gtk_widget_show (ControllerEditAnalogStickContentsL);
    gtk_table_attach (GTK_TABLE (ControllerEditAnalogStickFrameTable), ControllerEditAnalogStickContentsL, 3, 4, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);
    gtk_editable_set_editable (GTK_EDITABLE (ControllerEditAnalogStickContentsL), FALSE);
    gtk_entry_set_invisible_char (GTK_ENTRY (ControllerEditAnalogStickContentsL), 9679);

    ControllerEditAnalogStickContentsR = gtk_entry_new ();
    gtk_widget_show (ControllerEditAnalogStickContentsR);
    gtk_table_attach (GTK_TABLE (ControllerEditAnalogStickFrameTable), ControllerEditAnalogStickContentsR, 3, 4, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);
    gtk_editable_set_editable (GTK_EDITABLE (ControllerEditAnalogStickContentsR), FALSE);
    gtk_entry_set_invisible_char (GTK_ENTRY (ControllerEditAnalogStickContentsR), 9679);

    ControllerEditAnalogStickFrameLabel = gtk_label_new (tr("Analog Stick"));
    gtk_widget_show (ControllerEditAnalogStickFrameLabel);
    gtk_frame_set_label_widget (GTK_FRAME (ControllerEditAnalogStickFrame), ControllerEditAnalogStickFrameLabel);
    gtk_label_set_use_markup (GTK_LABEL (ControllerEditAnalogStickFrameLabel), TRUE);

    ControllerEditDPadFrame = gtk_frame_new (NULL);
    gtk_widget_show (ControllerEditDPadFrame);
    gtk_box_pack_start (GTK_BOX (ControllerEditDirectionalContainer), ControllerEditDPadFrame, TRUE, TRUE, 0);

    ControllerEditDPadFrameAlignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (ControllerEditDPadFrameAlignment);
    gtk_container_add (GTK_CONTAINER (ControllerEditDPadFrame), ControllerEditDPadFrameAlignment);
    gtk_alignment_set_padding (GTK_ALIGNMENT (ControllerEditDPadFrameAlignment), 0, 6, 6, 6);

    ControllerEditDPadFrameTable = gtk_table_new (2, 4, FALSE);
    gtk_widget_show (ControllerEditDPadFrameTable);
    gtk_container_add (GTK_CONTAINER (ControllerEditDPadFrameAlignment), ControllerEditDPadFrameTable);

    ControllerEditAnalogStickAssignU = gtk_button_new_with_mnemonic (tr("U"));
    gtk_widget_show (ControllerEditAnalogStickAssignU);
    gtk_table_attach (GTK_TABLE (ControllerEditDPadFrameTable), ControllerEditAnalogStickAssignU, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                       (GtkAttachOptions) (0), 0, 0);

    ControllerEditAnalogStickAssignD = gtk_button_new_with_mnemonic (tr("D"));
    gtk_widget_show (ControllerEditAnalogStickAssignD);
    gtk_table_attach (GTK_TABLE (ControllerEditDPadFrameTable), ControllerEditAnalogStickAssignD, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                       (GtkAttachOptions) (0), 0, 0);

    ControllerEditAnalogStickAssignL = gtk_button_new_with_mnemonic (tr("L"));
    gtk_widget_show (ControllerEditAnalogStickAssignL);
    gtk_table_attach (GTK_TABLE (ControllerEditDPadFrameTable), ControllerEditAnalogStickAssignL, 2, 3, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                       (GtkAttachOptions) (0), 0, 0);

    ControllerEditAnalogStickAssignR = gtk_button_new_with_mnemonic (tr("R"));
    gtk_widget_show (ControllerEditAnalogStickAssignR);
    gtk_table_attach (GTK_TABLE (ControllerEditDPadFrameTable), ControllerEditAnalogStickAssignR, 2, 3, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                       (GtkAttachOptions) (0), 0, 0);

    ControllerEditAnalogStickContentsU = gtk_entry_new ();
    gtk_widget_show (ControllerEditAnalogStickContentsU);
    gtk_table_attach (GTK_TABLE (ControllerEditDPadFrameTable), ControllerEditAnalogStickContentsU, 1, 2, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);
    gtk_editable_set_editable (GTK_EDITABLE (ControllerEditAnalogStickContentsU), FALSE);
    gtk_entry_set_invisible_char (GTK_ENTRY (ControllerEditAnalogStickContentsU), 9679);

    ControllerEditAnalogStickContentsD = gtk_entry_new ();
    gtk_widget_show (ControllerEditAnalogStickContentsD);
    gtk_table_attach (GTK_TABLE (ControllerEditDPadFrameTable), ControllerEditAnalogStickContentsD, 1, 2, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);
    gtk_editable_set_editable (GTK_EDITABLE (ControllerEditAnalogStickContentsD), FALSE);
    gtk_entry_set_invisible_char (GTK_ENTRY (ControllerEditAnalogStickContentsD), 9679);

    ControllerEditAnalogStickContentsL = gtk_entry_new ();
    gtk_widget_show (ControllerEditAnalogStickContentsL);
    gtk_table_attach (GTK_TABLE (ControllerEditDPadFrameTable), ControllerEditAnalogStickContentsL, 3, 4, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);
    gtk_editable_set_editable (GTK_EDITABLE (ControllerEditAnalogStickContentsL), FALSE);
    gtk_entry_set_invisible_char (GTK_ENTRY (ControllerEditAnalogStickContentsL), 9679);

    ControllerEditAnalogStickContentsR = gtk_entry_new ();
    gtk_widget_show (ControllerEditAnalogStickContentsR);
    gtk_table_attach (GTK_TABLE (ControllerEditDPadFrameTable), ControllerEditAnalogStickContentsR, 3, 4, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);
    gtk_editable_set_editable (GTK_EDITABLE (ControllerEditAnalogStickContentsR), FALSE);
    gtk_entry_set_invisible_char (GTK_ENTRY (ControllerEditAnalogStickContentsR), 9679);

    ControllerEditDPadFrameLabel = gtk_label_new (tr("Digital Pad"));
    gtk_widget_show (ControllerEditDPadFrameLabel);
    gtk_frame_set_label_widget (GTK_FRAME (ControllerEditDPadFrame), ControllerEditDPadFrameLabel);
    gtk_label_set_use_markup (GTK_LABEL (ControllerEditDPadFrameLabel), TRUE);

    ControllerEditCButtonsFrame = gtk_frame_new (NULL);
    gtk_widget_show (ControllerEditCButtonsFrame);
    gtk_box_pack_start (GTK_BOX (ControllerEditDirectionalContainer), ControllerEditCButtonsFrame, TRUE, TRUE, 0);

    ControllerEditCButtonsFrameAlignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (ControllerEditCButtonsFrameAlignment);
    gtk_container_add (GTK_CONTAINER (ControllerEditCButtonsFrame), ControllerEditCButtonsFrameAlignment);
    gtk_alignment_set_padding (GTK_ALIGNMENT (ControllerEditCButtonsFrameAlignment), 0, 6, 6, 6);

    ControllerEditCButtonsFrameTable = gtk_table_new (2, 4, FALSE);
    gtk_widget_show (ControllerEditCButtonsFrameTable);
    gtk_container_add (GTK_CONTAINER (ControllerEditCButtonsFrameAlignment), ControllerEditCButtonsFrameTable);

    ControllerEditCButtonsAssignU = gtk_button_new_with_mnemonic (tr("U"));
    gtk_widget_show (ControllerEditCButtonsAssignU);
    gtk_table_attach (GTK_TABLE (ControllerEditCButtonsFrameTable), ControllerEditCButtonsAssignU, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                       (GtkAttachOptions) (0), 0, 0);

    ControllerEditCButtonsAssignD = gtk_button_new_with_mnemonic (tr("D"));
    gtk_widget_show (ControllerEditCButtonsAssignD);
    gtk_table_attach (GTK_TABLE (ControllerEditCButtonsFrameTable), ControllerEditCButtonsAssignD, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                       (GtkAttachOptions) (0), 0, 0);

    ControllerEditCButtonsAssignL = gtk_button_new_with_mnemonic (tr("L"));
    gtk_widget_show (ControllerEditCButtonsAssignL);
    gtk_table_attach (GTK_TABLE (ControllerEditCButtonsFrameTable), ControllerEditCButtonsAssignL, 2, 3, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                       (GtkAttachOptions) (0), 0, 0);

    ControllerEditCButtonsAssignR = gtk_button_new_with_mnemonic (tr("R"));
    gtk_widget_show (ControllerEditCButtonsAssignR);
    gtk_table_attach (GTK_TABLE (ControllerEditCButtonsFrameTable), ControllerEditCButtonsAssignR, 2, 3, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                       (GtkAttachOptions) (0), 0, 0);

    ControllerEditCButtonsContentsU = gtk_entry_new ();
    gtk_widget_show (ControllerEditCButtonsContentsU);
    gtk_table_attach (GTK_TABLE (ControllerEditCButtonsFrameTable), ControllerEditCButtonsContentsU, 1, 2, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);
    gtk_editable_set_editable (GTK_EDITABLE (ControllerEditCButtonsContentsU), FALSE);
    gtk_entry_set_invisible_char (GTK_ENTRY (ControllerEditCButtonsContentsU), 9679);

    ControllerEditCButtonsContentsD = gtk_entry_new ();
    gtk_widget_show (ControllerEditCButtonsContentsD);
    gtk_table_attach (GTK_TABLE (ControllerEditCButtonsFrameTable), ControllerEditCButtonsContentsD, 1, 2, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);
    gtk_editable_set_editable (GTK_EDITABLE (ControllerEditCButtonsContentsD), FALSE);
    gtk_entry_set_invisible_char (GTK_ENTRY (ControllerEditCButtonsContentsD), 9679);

    ControllerEditCButtonsContentsL = gtk_entry_new ();
    gtk_widget_show (ControllerEditCButtonsContentsL);
    gtk_table_attach (GTK_TABLE (ControllerEditCButtonsFrameTable), ControllerEditCButtonsContentsL, 3, 4, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);
    gtk_editable_set_editable (GTK_EDITABLE (ControllerEditCButtonsContentsL), FALSE);
    gtk_entry_set_invisible_char (GTK_ENTRY (ControllerEditCButtonsContentsL), 9679);

    ControllerEditCButtonsContentsR = gtk_entry_new ();
    gtk_widget_show (ControllerEditCButtonsContentsR);
    gtk_table_attach (GTK_TABLE (ControllerEditCButtonsFrameTable), ControllerEditCButtonsContentsR, 3, 4, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);
    gtk_editable_set_editable (GTK_EDITABLE (ControllerEditCButtonsContentsR), FALSE);
    gtk_entry_set_invisible_char (GTK_ENTRY (ControllerEditCButtonsContentsR), 9679);

    ControllerEditCButtonsFrameLabel = gtk_label_new (tr("C Buttons"));
    gtk_widget_show (ControllerEditCButtonsFrameLabel);
    gtk_frame_set_label_widget (GTK_FRAME (ControllerEditCButtonsFrame), ControllerEditCButtonsFrameLabel);
    gtk_label_set_use_markup (GTK_LABEL (ControllerEditCButtonsFrameLabel), TRUE);

    ControllerEditSpeedModifiersFrame = gtk_frame_new (NULL);
    gtk_widget_show (ControllerEditSpeedModifiersFrame);
    gtk_fixed_put (GTK_FIXED (ControllerEditFixedContainer), ControllerEditSpeedModifiersFrame, 368, 112);
    gtk_widget_set_size_request (ControllerEditSpeedModifiersFrame, 177, 73);

    ControllerEditSpeedModifiersFrameAlignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (ControllerEditSpeedModifiersFrameAlignment);
    gtk_container_add (GTK_CONTAINER (ControllerEditSpeedModifiersFrame), ControllerEditSpeedModifiersFrameAlignment);
    gtk_alignment_set_padding (GTK_ALIGNMENT (ControllerEditSpeedModifiersFrameAlignment), 0, 6, 6, 6);

    ControllerEditSpeedModifiersFrameTable = gtk_table_new (2, 3, FALSE);
    gtk_widget_show (ControllerEditSpeedModifiersFrameTable);
    gtk_container_add (GTK_CONTAINER (ControllerEditSpeedModifiersFrameAlignment), ControllerEditSpeedModifiersFrameTable);

    ControllerEditSpeedModifiersAssignM1 = gtk_button_new_with_mnemonic (tr("M1"));
    gtk_widget_show (ControllerEditSpeedModifiersAssignM1);
    gtk_table_attach (GTK_TABLE (ControllerEditSpeedModifiersFrameTable), ControllerEditSpeedModifiersAssignM1, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                       (GtkAttachOptions) (0), 0, 0);

    ControllerEditSpeedModifiersAssignM2 = gtk_button_new_with_mnemonic (tr("M2"));
    gtk_widget_show (ControllerEditSpeedModifiersAssignM2);
    gtk_table_attach (GTK_TABLE (ControllerEditSpeedModifiersFrameTable), ControllerEditSpeedModifiersAssignM2, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                       (GtkAttachOptions) (0), 0, 0);

    ControllerEditSpeedModifiersContentsM1 = gtk_entry_new ();
    gtk_widget_show (ControllerEditSpeedModifiersContentsM1);
    gtk_table_attach (GTK_TABLE (ControllerEditSpeedModifiersFrameTable), ControllerEditSpeedModifiersContentsM1, 1, 2, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);
    gtk_editable_set_editable (GTK_EDITABLE (ControllerEditSpeedModifiersContentsM1), FALSE);
    gtk_entry_set_invisible_char (GTK_ENTRY (ControllerEditSpeedModifiersContentsM1), 9679);

    ControllerEditSpeedModifiersContentsM2 = gtk_entry_new ();
    gtk_widget_show (ControllerEditSpeedModifiersContentsM2);
    gtk_table_attach (GTK_TABLE (ControllerEditSpeedModifiersFrameTable), ControllerEditSpeedModifiersContentsM2, 1, 2, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);
    gtk_editable_set_editable (GTK_EDITABLE (ControllerEditSpeedModifiersContentsM2), FALSE);
    gtk_entry_set_invisible_char (GTK_ENTRY (ControllerEditSpeedModifiersContentsM2), 9679);

    ControllerEditSpeedModifiersSpinButtonM1_adj = gtk_adjustment_new (42, 0, 100, 1, 10, 10);
    ControllerEditSpeedModifiersSpinButtonM1 = gtk_spin_button_new (GTK_ADJUSTMENT (ControllerEditSpeedModifiersSpinButtonM1_adj), 1, 0);
    gtk_widget_show (ControllerEditSpeedModifiersSpinButtonM1);
    gtk_table_attach (GTK_TABLE (ControllerEditSpeedModifiersFrameTable), ControllerEditSpeedModifiersSpinButtonM1, 2, 3, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                       (GtkAttachOptions) (0), 0, 0);

    ControllerEditSpeedModifiersSpinButtonM2_adj = gtk_adjustment_new (20, 0, 100, 1, 10, 10);
    ControllerEditSpeedModifiersSpinButtonM2 = gtk_spin_button_new (GTK_ADJUSTMENT (ControllerEditSpeedModifiersSpinButtonM2_adj), 1, 0);
    gtk_widget_show (ControllerEditSpeedModifiersSpinButtonM2);
    gtk_table_attach (GTK_TABLE (ControllerEditSpeedModifiersFrameTable), ControllerEditSpeedModifiersSpinButtonM2, 2, 3, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                       (GtkAttachOptions) (0), 0, 0);

    ControllerEditSpeedModifiersFrameLabel = gtk_label_new (tr("Speed Modifiers"));
    gtk_widget_show (ControllerEditSpeedModifiersFrameLabel);
    gtk_frame_set_label_widget (GTK_FRAME (ControllerEditSpeedModifiersFrame), ControllerEditSpeedModifiersFrameLabel);
    gtk_label_set_use_markup (GTK_LABEL (ControllerEditSpeedModifiersFrameLabel), TRUE);

    ControllerEditMacrosFrame = gtk_frame_new (NULL);
    gtk_widget_show (ControllerEditMacrosFrame);
    gtk_fixed_put (GTK_FIXED (ControllerEditFixedContainer), ControllerEditMacrosFrame, 368, 192);
    gtk_widget_set_size_request (ControllerEditMacrosFrame, 177, 155);

    ControllerEditMacrosFrameAlignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (ControllerEditMacrosFrameAlignment);
    gtk_container_add (GTK_CONTAINER (ControllerEditMacrosFrame), ControllerEditMacrosFrameAlignment);
    gtk_alignment_set_padding (GTK_ALIGNMENT (ControllerEditMacrosFrameAlignment), 0, 6, 6, 6);

    ControllerEditMacrosFrameTable = gtk_table_new (5, 3, FALSE);
    gtk_widget_show (ControllerEditMacrosFrameTable);
    gtk_container_add (GTK_CONTAINER (ControllerEditMacrosFrameAlignment), ControllerEditMacrosFrameTable);

    ControllerEditMacrosAssignC1 = gtk_button_new_with_mnemonic (tr("C1"));
    gtk_widget_show (ControllerEditMacrosAssignC1);
    gtk_table_attach (GTK_TABLE (ControllerEditMacrosFrameTable), ControllerEditMacrosAssignC1, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                       (GtkAttachOptions) (0), 0, 0);

    ControllerEditMacrosAssignC2 = gtk_button_new_with_mnemonic (tr("C2"));
    gtk_widget_show (ControllerEditMacrosAssignC2);
    gtk_table_attach (GTK_TABLE (ControllerEditMacrosFrameTable), ControllerEditMacrosAssignC2, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                       (GtkAttachOptions) (0), 0, 0);

    ControllerEditMacrosAssignC3 = gtk_button_new_with_mnemonic (tr("C3"));
    gtk_widget_show (ControllerEditMacrosAssignC3);
    gtk_table_attach (GTK_TABLE (ControllerEditMacrosFrameTable), ControllerEditMacrosAssignC3, 0, 1, 2, 3,
                      (GtkAttachOptions) (GTK_FILL),
                       (GtkAttachOptions) (0), 0, 0);

    ControllerEditMacrosAssignC4 = gtk_button_new_with_mnemonic (tr("C4"));
    gtk_widget_show (ControllerEditMacrosAssignC4);
    gtk_table_attach (GTK_TABLE (ControllerEditMacrosFrameTable), ControllerEditMacrosAssignC4, 0, 1, 3, 4,
                      (GtkAttachOptions) (GTK_FILL),
                       (GtkAttachOptions) (0), 0, 0);

    ControllerEditMacrosAssignC5 = gtk_button_new_with_mnemonic (tr("C5"));
    gtk_widget_show (ControllerEditMacrosAssignC5);
    gtk_table_attach (GTK_TABLE (ControllerEditMacrosFrameTable), ControllerEditMacrosAssignC5, 0, 1, 4, 5,
                      (GtkAttachOptions) (GTK_FILL),
                       (GtkAttachOptions) (0), 0, 0);

    ControllerEditMacrosContentsC5 = gtk_entry_new ();
    gtk_widget_show (ControllerEditMacrosContentsC5);
    gtk_table_attach (GTK_TABLE (ControllerEditMacrosFrameTable), ControllerEditMacrosContentsC5, 1, 2, 4, 5,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);
    gtk_editable_set_editable (GTK_EDITABLE (ControllerEditMacrosContentsC5), FALSE);
    gtk_entry_set_invisible_char (GTK_ENTRY (ControllerEditMacrosContentsC5), 9679);

    ControllerEditMacrosContentsC4 = gtk_entry_new ();
    gtk_widget_show (ControllerEditMacrosContentsC4);
    gtk_table_attach (GTK_TABLE (ControllerEditMacrosFrameTable), ControllerEditMacrosContentsC4, 1, 2, 3, 4,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);
    gtk_editable_set_editable (GTK_EDITABLE (ControllerEditMacrosContentsC4), FALSE);
    gtk_entry_set_invisible_char (GTK_ENTRY (ControllerEditMacrosContentsC4), 9679);

    ControllerEditMacrosContentsC3 = gtk_entry_new ();
    gtk_widget_show (ControllerEditMacrosContentsC3);
    gtk_table_attach (GTK_TABLE (ControllerEditMacrosFrameTable), ControllerEditMacrosContentsC3, 1, 2, 2, 3,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);
    gtk_editable_set_editable (GTK_EDITABLE (ControllerEditMacrosContentsC3), FALSE);
    gtk_entry_set_invisible_char (GTK_ENTRY (ControllerEditMacrosContentsC3), 9679);

    ControllerEditMacrosContentsC2 = gtk_entry_new ();
    gtk_widget_show (ControllerEditMacrosContentsC2);
    gtk_table_attach (GTK_TABLE (ControllerEditMacrosFrameTable), ControllerEditMacrosContentsC2, 1, 2, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);
    gtk_editable_set_editable (GTK_EDITABLE (ControllerEditMacrosContentsC2), FALSE);
    gtk_entry_set_invisible_char (GTK_ENTRY (ControllerEditMacrosContentsC2), 9679);

    ControllerEditMacrosContentsC1 = gtk_entry_new ();
    gtk_widget_show (ControllerEditMacrosContentsC1);
    gtk_table_attach (GTK_TABLE (ControllerEditMacrosFrameTable), ControllerEditMacrosContentsC1, 1, 2, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);
    gtk_editable_set_editable (GTK_EDITABLE (ControllerEditMacrosContentsC1), FALSE);
    gtk_entry_set_invisible_char (GTK_ENTRY (ControllerEditMacrosContentsC1), 9679);

    ControllerEditMacrosEditC1 = gtk_button_new_with_mnemonic (tr("Edit"));
    gtk_widget_show (ControllerEditMacrosEditC1);
    gtk_table_attach (GTK_TABLE (ControllerEditMacrosFrameTable), ControllerEditMacrosEditC1, 2, 3, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                       (GtkAttachOptions) (0), 0, 0);

    ControllerEditMacrosEditC2 = gtk_button_new_with_mnemonic (tr("Edit"));
    gtk_widget_show (ControllerEditMacrosEditC2);
    gtk_table_attach (GTK_TABLE (ControllerEditMacrosFrameTable), ControllerEditMacrosEditC2, 2, 3, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                       (GtkAttachOptions) (0), 0, 0);

    ControllerEditMacrosEditC3 = gtk_button_new_with_mnemonic (tr("Edit"));
    gtk_widget_show (ControllerEditMacrosEditC3);
    gtk_table_attach (GTK_TABLE (ControllerEditMacrosFrameTable), ControllerEditMacrosEditC3, 2, 3, 2, 3,
                      (GtkAttachOptions) (GTK_FILL),
                       (GtkAttachOptions) (0), 0, 0);

    ControllerEditMacrosEditC4 = gtk_button_new_with_mnemonic (tr("Edit"));
    gtk_widget_show (ControllerEditMacrosEditC4);
    gtk_table_attach (GTK_TABLE (ControllerEditMacrosFrameTable), ControllerEditMacrosEditC4, 2, 3, 3, 4,
                      (GtkAttachOptions) (GTK_FILL),
                       (GtkAttachOptions) (0), 0, 0);

    ControllerEditMacrosEditC5 = gtk_button_new_with_mnemonic (tr("Edit"));
    gtk_widget_show (ControllerEditMacrosEditC5);
    gtk_table_attach (GTK_TABLE (ControllerEditMacrosFrameTable), ControllerEditMacrosEditC5, 2, 3, 4, 5,
                      (GtkAttachOptions) (GTK_FILL),
                       (GtkAttachOptions) (0), 0, 0);

    ControllerEditMacrosFrameLabel = gtk_label_new (tr("Macros"));
    gtk_widget_show (ControllerEditMacrosFrameLabel);
    gtk_frame_set_label_widget (GTK_FRAME (ControllerEditMacrosFrame), ControllerEditMacrosFrameLabel);
    gtk_label_set_use_markup (GTK_LABEL (ControllerEditMacrosFrameLabel), TRUE);

    ControllerEditButtonStrip = gtk_hbutton_box_new ();
    gtk_widget_show (ControllerEditButtonStrip);
    gtk_fixed_put (GTK_FIXED (ControllerEditFixedContainer), ControllerEditButtonStrip, 8, 352);
    gtk_widget_set_size_request (ControllerEditButtonStrip, 535, 27);

    ControllerEditReset = gtk_button_new_with_mnemonic (tr("Reset"));
    gtk_widget_show (ControllerEditReset);
    gtk_container_add (GTK_CONTAINER (ControllerEditButtonStrip), ControllerEditReset);
    GTK_WIDGET_SET_FLAGS (ControllerEditReset, GTK_CAN_DEFAULT);

    ControllerEditLoadDef = gtk_button_new_with_mnemonic (tr("Load Def."));
    gtk_widget_show (ControllerEditLoadDef);
    gtk_container_add (GTK_CONTAINER (ControllerEditButtonStrip), ControllerEditLoadDef);
    GTK_WIDGET_SET_FLAGS (ControllerEditLoadDef, GTK_CAN_DEFAULT);

    ControllerEditSaveDef = gtk_button_new_with_mnemonic (tr("Save Def."));
    gtk_widget_show (ControllerEditSaveDef);
    gtk_container_add (GTK_CONTAINER (ControllerEditButtonStrip), ControllerEditSaveDef);
    GTK_WIDGET_SET_FLAGS (ControllerEditSaveDef, GTK_CAN_DEFAULT);

    ControllerEditApply = gtk_button_new_from_stock ("gtk-apply");
    gtk_widget_show (ControllerEditApply);
    gtk_container_add (GTK_CONTAINER (ControllerEditButtonStrip), ControllerEditApply);
    GTK_WIDGET_SET_FLAGS (ControllerEditApply, GTK_CAN_DEFAULT);

    ControllerEditOK = gtk_button_new_from_stock ("gtk-ok");
    gtk_widget_show (ControllerEditOK);
    gtk_container_add (GTK_CONTAINER (ControllerEditButtonStrip), ControllerEditOK);
    GTK_WIDGET_SET_FLAGS (ControllerEditOK, GTK_CAN_DEFAULT);

    ControllerEditCancel = gtk_button_new_from_stock ("gtk-cancel");
    gtk_widget_show (ControllerEditCancel);
    gtk_container_add (GTK_CONTAINER (ControllerEditButtonStrip), ControllerEditCancel);
    GTK_WIDGET_SET_FLAGS (ControllerEditCancel, GTK_CAN_DEFAULT);

    ControllerEditCurrentControllerLabel = gtk_label_new (tr("Current N64 Controller"));
    gtk_widget_show (ControllerEditCurrentControllerLabel);
    gtk_fixed_put (GTK_FIXED (ControllerEditFixedContainer), ControllerEditCurrentControllerLabel, 8, 8);
    gtk_widget_set_size_request (ControllerEditCurrentControllerLabel, 128, 16);
    gtk_misc_set_alignment (GTK_MISC (ControllerEditCurrentControllerLabel), 0, 0.5);

    ControllerEditButtonsFrame = gtk_frame_new (NULL);
    gtk_widget_show (ControllerEditButtonsFrame);
    gtk_fixed_put (GTK_FIXED (ControllerEditFixedContainer), ControllerEditButtonsFrame, 8, 144);
    gtk_widget_set_size_request (ControllerEditButtonsFrame, 104, 101);

    ControllerEditButtonsFrameAlignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (ControllerEditButtonsFrameAlignment);
    gtk_container_add (GTK_CONTAINER (ControllerEditButtonsFrame), ControllerEditButtonsFrameAlignment);
    gtk_alignment_set_padding (GTK_ALIGNMENT (ControllerEditButtonsFrameAlignment), 0, 6, 6, 6);

    ControllerEditButtonsFrameTable = gtk_table_new (3, 2, FALSE);
    gtk_widget_show (ControllerEditButtonsFrameTable);
    gtk_container_add (GTK_CONTAINER (ControllerEditButtonsFrameAlignment), ControllerEditButtonsFrameTable);

    ControllerEditButtonsAssignS = gtk_button_new_with_mnemonic (tr("S"));
    gtk_widget_show (ControllerEditButtonsAssignS);
    gtk_table_attach (GTK_TABLE (ControllerEditButtonsFrameTable), ControllerEditButtonsAssignS, 0, 1, 2, 3,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND), 0, 0);

    ControllerEditButtonsAssignB = gtk_button_new_with_mnemonic (tr("B"));
    gtk_widget_show (ControllerEditButtonsAssignB);
    gtk_table_attach (GTK_TABLE (ControllerEditButtonsFrameTable), ControllerEditButtonsAssignB, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND), 0, 0);

    ControllerEditButtonsAssignA = gtk_button_new_with_mnemonic (tr("A"));
    gtk_widget_show (ControllerEditButtonsAssignA);
    gtk_table_attach (GTK_TABLE (ControllerEditButtonsFrameTable), ControllerEditButtonsAssignA, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND), 0, 0);

    ControllerEditButtonsContentsA = gtk_entry_new ();
    gtk_widget_show (ControllerEditButtonsContentsA);
    gtk_table_attach (GTK_TABLE (ControllerEditButtonsFrameTable), ControllerEditButtonsContentsA, 1, 2, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);
    gtk_editable_set_editable (GTK_EDITABLE (ControllerEditButtonsContentsA), FALSE);
    gtk_entry_set_invisible_char (GTK_ENTRY (ControllerEditButtonsContentsA), 9679);

    ControllerEditButtonsContentsB = gtk_entry_new ();
    gtk_widget_show (ControllerEditButtonsContentsB);
    gtk_table_attach (GTK_TABLE (ControllerEditButtonsFrameTable), ControllerEditButtonsContentsB, 1, 2, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);
    gtk_editable_set_editable (GTK_EDITABLE (ControllerEditButtonsContentsB), FALSE);
    gtk_entry_set_invisible_char (GTK_ENTRY (ControllerEditButtonsContentsB), 9679);

    ControllerEditButtonsContentsS = gtk_entry_new ();
    gtk_widget_show (ControllerEditButtonsContentsS);
    gtk_table_attach (GTK_TABLE (ControllerEditButtonsFrameTable), ControllerEditButtonsContentsS, 1, 2, 2, 3,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);
    gtk_editable_set_editable (GTK_EDITABLE (ControllerEditButtonsContentsS), FALSE);
    gtk_entry_set_invisible_char (GTK_ENTRY (ControllerEditButtonsContentsS), 9679);

    ControllerEditButtonsFrameLabel = gtk_label_new (tr("Buttons"));
    gtk_widget_show (ControllerEditButtonsFrameLabel);
    gtk_frame_set_label_widget (GTK_FRAME (ControllerEditButtonsFrame), ControllerEditButtonsFrameLabel);
    gtk_label_set_use_markup (GTK_LABEL (ControllerEditButtonsFrameLabel), TRUE);

    ControllerEditTriggersFrame = gtk_frame_new (NULL);
    gtk_widget_show (ControllerEditTriggersFrame);
    gtk_fixed_put (GTK_FIXED (ControllerEditFixedContainer), ControllerEditTriggersFrame, 8, 248);
    gtk_widget_set_size_request (ControllerEditTriggersFrame, 104, 101);

    ControllerEditTriggersFrameAlignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (ControllerEditTriggersFrameAlignment);
    gtk_container_add (GTK_CONTAINER (ControllerEditTriggersFrame), ControllerEditTriggersFrameAlignment);
    gtk_alignment_set_padding (GTK_ALIGNMENT (ControllerEditTriggersFrameAlignment), 0, 6, 6, 6);

    ControllerEditTriggersFrameTable = gtk_table_new (3, 2, FALSE);
    gtk_widget_show (ControllerEditTriggersFrameTable);
    gtk_container_add (GTK_CONTAINER (ControllerEditTriggersFrameAlignment), ControllerEditTriggersFrameTable);

    ControllerEditTriggersAssignZ = gtk_button_new_with_mnemonic (tr("Z"));
    gtk_widget_show (ControllerEditTriggersAssignZ);
    gtk_table_attach (GTK_TABLE (ControllerEditTriggersFrameTable), ControllerEditTriggersAssignZ, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND), 0, 0);

    ControllerEditTriggersAssignL = gtk_button_new_with_mnemonic (tr("L"));
    gtk_widget_show (ControllerEditTriggersAssignL);
    gtk_table_attach (GTK_TABLE (ControllerEditTriggersFrameTable), ControllerEditTriggersAssignL, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND), 0, 0);

    ControllerEditTriggersAssignR = gtk_button_new_with_mnemonic (tr("R"));
    gtk_widget_show (ControllerEditTriggersAssignR);
    gtk_table_attach (GTK_TABLE (ControllerEditTriggersFrameTable), ControllerEditTriggersAssignR, 0, 1, 2, 3,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND), 0, 0);

    ControllerEditTriggersContentsZ = gtk_entry_new ();
    gtk_widget_show (ControllerEditTriggersContentsZ);
    gtk_table_attach (GTK_TABLE (ControllerEditTriggersFrameTable), ControllerEditTriggersContentsZ, 1, 2, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);
    gtk_editable_set_editable (GTK_EDITABLE (ControllerEditTriggersContentsZ), FALSE);
    gtk_entry_set_invisible_char (GTK_ENTRY (ControllerEditTriggersContentsZ), 9679);

    ControlllerEditTriggersContentsL = gtk_entry_new ();
    gtk_widget_show (ControlllerEditTriggersContentsL);
    gtk_table_attach (GTK_TABLE (ControllerEditTriggersFrameTable), ControlllerEditTriggersContentsL, 1, 2, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);
    gtk_editable_set_editable (GTK_EDITABLE (ControlllerEditTriggersContentsL), FALSE);
    gtk_entry_set_invisible_char (GTK_ENTRY (ControlllerEditTriggersContentsL), 9679);

    ControllerEditTriggersContentsR = gtk_entry_new ();
    gtk_widget_show (ControllerEditTriggersContentsR);
    gtk_table_attach (GTK_TABLE (ControllerEditTriggersFrameTable), ControllerEditTriggersContentsR, 1, 2, 2, 3,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);
    gtk_editable_set_editable (GTK_EDITABLE (ControllerEditTriggersContentsR), FALSE);
    gtk_entry_set_invisible_char (GTK_ENTRY (ControllerEditTriggersContentsR), 9679);

    ControllerEditTriggersFrameLabel = gtk_label_new (tr("Triggers"));
    gtk_widget_show (ControllerEditTriggersFrameLabel);
    gtk_frame_set_label_widget (GTK_FRAME (ControllerEditTriggersFrame), ControllerEditTriggersFrameLabel);
    gtk_label_set_use_markup (GTK_LABEL (ControllerEditTriggersFrameLabel), TRUE);

    ControllerEditCurrentControllerCombobox = gtk_combo_box_new_text ();
    gtk_widget_show (ControllerEditCurrentControllerCombobox);
    gtk_fixed_put (GTK_FIXED (ControllerEditFixedContainer), ControllerEditCurrentControllerCombobox, 8, 24);
    gtk_widget_set_size_request (ControllerEditCurrentControllerCombobox, 106, 28);
    gtk_container_set_border_width (GTK_CONTAINER (ControllerEditCurrentControllerCombobox), 1);
    gtk_combo_box_append_text (GTK_COMBO_BOX (ControllerEditCurrentControllerCombobox), tr("Controller 1"));
    gtk_combo_box_append_text (GTK_COMBO_BOX (ControllerEditCurrentControllerCombobox), tr("Controller 2"));
    gtk_combo_box_append_text (GTK_COMBO_BOX (ControllerEditCurrentControllerCombobox), tr("Controller 3"));
    gtk_combo_box_append_text (GTK_COMBO_BOX (ControllerEditCurrentControllerCombobox), tr("Controller 4"));

    ControllerEditSensitivityFrame = gtk_frame_new (NULL);
    gtk_widget_show (ControllerEditSensitivityFrame);
    gtk_fixed_put (GTK_FIXED (ControllerEditFixedContainer), ControllerEditSensitivityFrame, 8, 64);
    gtk_widget_set_size_request (ControllerEditSensitivityFrame, 104, 80);

    ControllerEditSensitivityFrameAlignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (ControllerEditSensitivityFrameAlignment);
    gtk_container_add (GTK_CONTAINER (ControllerEditSensitivityFrame), ControllerEditSensitivityFrameAlignment);
    gtk_alignment_set_padding (GTK_ALIGNMENT (ControllerEditSensitivityFrameAlignment), 0, 6, 6, 6);

    ControllerEditSensitivityFrameTable = gtk_table_new (2, 2, FALSE);
    gtk_widget_show (ControllerEditSensitivityFrameTable);
    gtk_container_add (GTK_CONTAINER (ControllerEditSensitivityFrameAlignment), ControllerEditSensitivityFrameTable);

    ControllerEditSensitivityMinLabel = gtk_label_new (tr("Min"));
    gtk_widget_show (ControllerEditSensitivityMinLabel);
    gtk_table_attach (GTK_TABLE (ControllerEditSensitivityFrameTable), ControllerEditSensitivityMinLabel, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (ControllerEditSensitivityMinLabel), 0, 0.5);

    ControllerEditSensitivityMaxLabel = gtk_label_new (tr("Max:"));
    gtk_widget_show (ControllerEditSensitivityMaxLabel);
    gtk_table_attach (GTK_TABLE (ControllerEditSensitivityFrameTable), ControllerEditSensitivityMaxLabel, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (ControllerEditSensitivityMaxLabel), 0, 0.5);

    ControllerEditSensitivityMaxSpinbox_adj = gtk_adjustment_new (128, 50, 181, 1, 10, 10);
    ControllerEditSensitivityMaxSpinbox = gtk_spin_button_new (GTK_ADJUSTMENT (ControllerEditSensitivityMaxSpinbox_adj), 1, 0);
    gtk_widget_show (ControllerEditSensitivityMaxSpinbox);
    gtk_table_attach (GTK_TABLE (ControllerEditSensitivityFrameTable), ControllerEditSensitivityMaxSpinbox, 1, 2, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND), 0, 0);

    ControllerEditSensitivityMinSpinbox_adj = gtk_adjustment_new (32, 0, 50, 1, 10, 10);
    ControllerEditSensitivityMinSpinbox = gtk_spin_button_new (GTK_ADJUSTMENT (ControllerEditSensitivityMinSpinbox_adj), 1, 0);
    gtk_widget_show (ControllerEditSensitivityMinSpinbox);
    gtk_table_attach (GTK_TABLE (ControllerEditSensitivityFrameTable), ControllerEditSensitivityMinSpinbox, 1, 2, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND), 0, 0);

    ControllerEditSensitivityFrameLabel = gtk_label_new (tr("Sensitivity"));
    gtk_widget_show (ControllerEditSensitivityFrameLabel);
    gtk_frame_set_label_widget (GTK_FRAME (ControllerEditSensitivityFrame), ControllerEditSensitivityFrameLabel);
    gtk_label_set_use_markup (GTK_LABEL (ControllerEditSensitivityFrameLabel), TRUE);

    ControllerEditPropertiesFrame = gtk_frame_new (NULL);
    gtk_widget_show (ControllerEditPropertiesFrame);
    gtk_fixed_put (GTK_FIXED (ControllerEditFixedContainer), ControllerEditPropertiesFrame, 128, 40);
    gtk_widget_set_size_request (ControllerEditPropertiesFrame, 90, 60);
    gtk_frame_set_shadow_type (GTK_FRAME (ControllerEditPropertiesFrame), GTK_SHADOW_OUT);

    ControllerEditPropertiesFrameAlignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (ControllerEditPropertiesFrameAlignment);
    gtk_container_add (GTK_CONTAINER (ControllerEditPropertiesFrame), ControllerEditPropertiesFrameAlignment);
    gtk_alignment_set_padding (GTK_ALIGNMENT (ControllerEditPropertiesFrameAlignment), 0, 0, 6, 6);

    ControllerEditProperties = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (ControllerEditProperties);
    gtk_container_add (GTK_CONTAINER (ControllerEditPropertiesFrameAlignment), ControllerEditProperties);

    ControllerEditPropertiesActiveCheckbox = gtk_check_button_new_with_mnemonic (tr("Active"));
    gtk_widget_show (ControllerEditPropertiesActiveCheckbox);
    gtk_box_pack_start (GTK_BOX (ControllerEditProperties), ControllerEditPropertiesActiveCheckbox, FALSE, FALSE, 0);

    ControllerEditPropertiesMemPakCheckbox = gtk_check_button_new_with_mnemonic (tr("MemPak"));
    gtk_widget_show (ControllerEditPropertiesMemPakCheckbox);
    gtk_box_pack_start (GTK_BOX (ControllerEditProperties), ControllerEditPropertiesMemPakCheckbox, FALSE, FALSE, 0);

    ControllerEditPropertiesFrameLabel = gtk_label_new (tr("Properties"));
    gtk_widget_show (ControllerEditPropertiesFrameLabel);
    gtk_frame_set_label_widget (GTK_FRAME (ControllerEditPropertiesFrame), ControllerEditPropertiesFrameLabel);
    gtk_label_set_use_markup (GTK_LABEL (ControllerEditPropertiesFrameLabel), TRUE);

    /* Store pointers to all widgets, for use by lookup_widget(). */
    GTKHUOBJECT_NO_REF (ControllerEdit, ControllerEdit, "ControllerEdit");
    GTKHUOBJECT (ControllerEdit, ControllerEditFixedContainer, "ControllerEditFixedContainer");
    GTKHUOBJECT (ControllerEdit, ControllerEditDeviceListFrame, "ControllerEditDeviceListFrame");
    GTKHUOBJECT (ControllerEdit, ControllerEditDeviceListFrameAlignment, "ControllerEditDeviceListFrameAlignment");
    GTKHUOBJECT (ControllerEdit, ControllerEditDeviceListDeviceWindow, "ControllerEditDeviceListDeviceWindow");
    GTKHUOBJECT (ControllerEdit, ControllerEditDeviceListDeviceView, "ControllerEditDeviceListDeviceView");
    GTKHUOBJECT (ControllerEdit, ControllerEditDeviceListFrameLabel, "ControllerEditDeviceListFrameLabel");
    GTKHUOBJECT (ControllerEdit, ControllerEditDirectionalContainer, "ControllerEditDirectionalContainer");
    GTKHUOBJECT (ControllerEdit, ControllerEditAnalogStickFrame, "ControllerEditAnalogStickFrame");
    GTKHUOBJECT (ControllerEdit, ControllerEditAnaologStickFrameAlignment, "ControllerEditAnaologStickFrameAlignment");
    GTKHUOBJECT (ControllerEdit, ControllerEditAnalogStickFrameTable, "ControllerEditAnalogStickFrameTable");
    GTKHUOBJECT (ControllerEdit, ControllerEditAnalogStickAssignU, "ControllerEditAnalogStickAssignU");
    GTKHUOBJECT (ControllerEdit, ControllerEditAnalogStickAssignD, "ControllerEditAnalogStickAssignD");
    GTKHUOBJECT (ControllerEdit, ControllerEditAnalogStickAssignL, "ControllerEditAnalogStickAssignL");
    GTKHUOBJECT (ControllerEdit, ControllerEditAnalogStickAssignR, "ControllerEditAnalogStickAssignR");
    GTKHUOBJECT (ControllerEdit, ControllerEditAnalogStickContentsU, "ControllerEditAnalogStickContentsU");
    GTKHUOBJECT (ControllerEdit, ControllerEditAnalogStickContentsD, "ControllerEditAnalogStickContentsD");
    GTKHUOBJECT (ControllerEdit, ControllerEditAnalogStickContentsL, "ControllerEditAnalogStickContentsL");
    GTKHUOBJECT (ControllerEdit, ControllerEditAnalogStickContentsR, "ControllerEditAnalogStickContentsR");
    GTKHUOBJECT (ControllerEdit, ControllerEditAnalogStickFrameLabel, "ControllerEditAnalogStickFrameLabel");
    GTKHUOBJECT (ControllerEdit, ControllerEditDPadFrame, "ControllerEditDPadFrame");
    GTKHUOBJECT (ControllerEdit, ControllerEditDPadFrameAlignment, "ControllerEditDPadFrameAlignment");
    GTKHUOBJECT (ControllerEdit, ControllerEditDPadFrameTable, "ControllerEditDPadFrameTable");
    GTKHUOBJECT (ControllerEdit, ControllerEditAnalogStickAssignU, "ControllerEditAnalogStickAssignU");
    GTKHUOBJECT (ControllerEdit, ControllerEditAnalogStickAssignD, "ControllerEditAnalogStickAssignD");
    GTKHUOBJECT (ControllerEdit, ControllerEditAnalogStickAssignL, "ControllerEditAnalogStickAssignL");
    GTKHUOBJECT (ControllerEdit, ControllerEditAnalogStickAssignR, "ControllerEditAnalogStickAssignR");
    GTKHUOBJECT (ControllerEdit, ControllerEditAnalogStickContentsU, "ControllerEditAnalogStickContentsU");
    GTKHUOBJECT (ControllerEdit, ControllerEditAnalogStickContentsD, "ControllerEditAnalogStickContentsD");
    GTKHUOBJECT (ControllerEdit, ControllerEditAnalogStickContentsL, "ControllerEditAnalogStickContentsL");
    GTKHUOBJECT (ControllerEdit, ControllerEditAnalogStickContentsR, "ControllerEditAnalogStickContentsR");
    GTKHUOBJECT (ControllerEdit, ControllerEditDPadFrameLabel, "ControllerEditDPadFrameLabel");
    GTKHUOBJECT (ControllerEdit, ControllerEditCButtonsFrame, "ControllerEditCButtonsFrame");
    GTKHUOBJECT (ControllerEdit, ControllerEditCButtonsFrameAlignment, "ControllerEditCButtonsFrameAlignment");
    GTKHUOBJECT (ControllerEdit, ControllerEditCButtonsFrameTable, "ControllerEditCButtonsFrameTable");
    GTKHUOBJECT (ControllerEdit, ControllerEditCButtonsAssignU, "ControllerEditCButtonsAssignU");
    GTKHUOBJECT (ControllerEdit, ControllerEditCButtonsAssignD, "ControllerEditCButtonsAssignD");
    GTKHUOBJECT (ControllerEdit, ControllerEditCButtonsAssignL, "ControllerEditCButtonsAssignL");
    GTKHUOBJECT (ControllerEdit, ControllerEditCButtonsAssignR, "ControllerEditCButtonsAssignR");
    GTKHUOBJECT (ControllerEdit, ControllerEditCButtonsContentsU, "ControllerEditCButtonsContentsU");
    GTKHUOBJECT (ControllerEdit, ControllerEditCButtonsContentsD, "ControllerEditCButtonsContentsD");
    GTKHUOBJECT (ControllerEdit, ControllerEditCButtonsContentsL, "ControllerEditCButtonsContentsL");
    GTKHUOBJECT (ControllerEdit, ControllerEditCButtonsContentsR, "ControllerEditCButtonsContentsR");
    GTKHUOBJECT (ControllerEdit, ControllerEditCButtonsFrameLabel, "ControllerEditCButtonsFrameLabel");
    GTKHUOBJECT (ControllerEdit, ControllerEditSpeedModifiersFrame, "ControllerEditSpeedModifiersFrame");
    GTKHUOBJECT (ControllerEdit, ControllerEditSpeedModifiersFrameAlignment, "ControllerEditSpeedModifiersFrameAlignment");
    GTKHUOBJECT (ControllerEdit, ControllerEditSpeedModifiersFrameTable, "ControllerEditSpeedModifiersFrameTable");
    GTKHUOBJECT (ControllerEdit, ControllerEditSpeedModifiersAssignM1, "ControllerEditSpeedModifiersAssignM1");
    GTKHUOBJECT (ControllerEdit, ControllerEditSpeedModifiersAssignM2, "ControllerEditSpeedModifiersAssignM2");
    GTKHUOBJECT (ControllerEdit, ControllerEditSpeedModifiersContentsM1, "ControllerEditSpeedModifiersContentsM1");
    GTKHUOBJECT (ControllerEdit, ControllerEditSpeedModifiersContentsM2, "ControllerEditSpeedModifiersContentsM2");
    GTKHUOBJECT (ControllerEdit, ControllerEditSpeedModifiersSpinButtonM1, "ControllerEditSpeedModifiersSpinButtonM1");
    GTKHUOBJECT (ControllerEdit, ControllerEditSpeedModifiersSpinButtonM2, "ControllerEditSpeedModifiersSpinButtonM2");
    GTKHUOBJECT (ControllerEdit, ControllerEditSpeedModifiersFrameLabel, "ControllerEditSpeedModifiersFrameLabel");
    GTKHUOBJECT (ControllerEdit, ControllerEditMacrosFrame, "ControllerEditMacrosFrame");
    GTKHUOBJECT (ControllerEdit, ControllerEditMacrosFrameAlignment, "ControllerEditMacrosFrameAlignment");
    GTKHUOBJECT (ControllerEdit, ControllerEditMacrosFrameTable, "ControllerEditMacrosFrameTable");
    GTKHUOBJECT (ControllerEdit, ControllerEditMacrosAssignC1, "ControllerEditMacrosAssignC1");
    GTKHUOBJECT (ControllerEdit, ControllerEditMacrosAssignC2, "ControllerEditMacrosAssignC2");
    GTKHUOBJECT (ControllerEdit, ControllerEditMacrosAssignC3, "ControllerEditMacrosAssignC3");
    GTKHUOBJECT (ControllerEdit, ControllerEditMacrosAssignC4, "ControllerEditMacrosAssignC4");
    GTKHUOBJECT (ControllerEdit, ControllerEditMacrosAssignC5, "ControllerEditMacrosAssignC5");
    GTKHUOBJECT (ControllerEdit, ControllerEditMacrosContentsC5, "ControllerEditMacrosContentsC5");
    GTKHUOBJECT (ControllerEdit, ControllerEditMacrosContentsC4, "ControllerEditMacrosContentsC4");
    GTKHUOBJECT (ControllerEdit, ControllerEditMacrosContentsC3, "ControllerEditMacrosContentsC3");
    GTKHUOBJECT (ControllerEdit, ControllerEditMacrosContentsC2, "ControllerEditMacrosContentsC2");
    GTKHUOBJECT (ControllerEdit, ControllerEditMacrosContentsC1, "ControllerEditMacrosContentsC1");
    GTKHUOBJECT (ControllerEdit, ControllerEditMacrosEditC1, "ControllerEditMacrosEditC1");
    GTKHUOBJECT (ControllerEdit, ControllerEditMacrosEditC2, "ControllerEditMacrosEditC2");
    GTKHUOBJECT (ControllerEdit, ControllerEditMacrosEditC3, "ControllerEditMacrosEditC3");
    GTKHUOBJECT (ControllerEdit, ControllerEditMacrosEditC4, "ControllerEditMacrosEditC4");
    GTKHUOBJECT (ControllerEdit, ControllerEditMacrosEditC5, "ControllerEditMacrosEditC5");
    GTKHUOBJECT (ControllerEdit, ControllerEditMacrosFrameLabel, "ControllerEditMacrosFrameLabel");
    GTKHUOBJECT (ControllerEdit, ControllerEditButtonStrip, "ControllerEditButtonStrip");
    GTKHUOBJECT (ControllerEdit, ControllerEditReset, "ControllerEditReset");
    GTKHUOBJECT (ControllerEdit, ControllerEditLoadDef, "ControllerEditLoadDef");
    GTKHUOBJECT (ControllerEdit, ControllerEditSaveDef, "ControllerEditSaveDef");
    GTKHUOBJECT (ControllerEdit, ControllerEditApply, "ControllerEditApply");
    GTKHUOBJECT (ControllerEdit, ControllerEditOK, "ControllerEditOK");
    GTKHUOBJECT (ControllerEdit, ControllerEditCancel, "ControllerEditCancel");
    GTKHUOBJECT (ControllerEdit, ControllerEditCurrentControllerLabel, "ControllerEditCurrentControllerLabel");
    GTKHUOBJECT (ControllerEdit, ControllerEditButtonsFrame, "ControllerEditButtonsFrame");
    GTKHUOBJECT (ControllerEdit, ControllerEditButtonsFrameAlignment, "ControllerEditButtonsFrameAlignment");
    GTKHUOBJECT (ControllerEdit, ControllerEditButtonsFrameTable, "ControllerEditButtonsFrameTable");
    GTKHUOBJECT (ControllerEdit, ControllerEditButtonsAssignS, "ControllerEditButtonsAssignS");
    GTKHUOBJECT (ControllerEdit, ControllerEditButtonsAssignB, "ControllerEditButtonsAssignB");
    GTKHUOBJECT (ControllerEdit, ControllerEditButtonsAssignA, "ControllerEditButtonsAssignA");
    GTKHUOBJECT (ControllerEdit, ControllerEditButtonsContentsA, "ControllerEditButtonsContentsA");
    GTKHUOBJECT (ControllerEdit, ControllerEditButtonsContentsB, "ControllerEditButtonsContentsB");
    GTKHUOBJECT (ControllerEdit, ControllerEditButtonsContentsS, "ControllerEditButtonsContentsS");
    GTKHUOBJECT (ControllerEdit, ControllerEditButtonsFrameLabel, "ControllerEditButtonsFrameLabel");
    GTKHUOBJECT (ControllerEdit, ControllerEditTriggersFrame, "ControllerEditTriggersFrame");
    GTKHUOBJECT (ControllerEdit, ControllerEditTriggersFrameAlignment, "ControllerEditTriggersFrameAlignment");
    GTKHUOBJECT (ControllerEdit, ControllerEditTriggersFrameTable, "ControllerEditTriggersFrameTable");
    GTKHUOBJECT (ControllerEdit, ControllerEditTriggersAssignZ, "ControllerEditTriggersAssignZ");
    GTKHUOBJECT (ControllerEdit, ControllerEditTriggersAssignL, "ControllerEditTriggersAssignL");
    GTKHUOBJECT (ControllerEdit, ControllerEditTriggersAssignR, "ControllerEditTriggersAssignR");
    GTKHUOBJECT (ControllerEdit, ControllerEditTriggersContentsZ, "ControllerEditTriggersContentsZ");
    GTKHUOBJECT (ControllerEdit, ControlllerEditTriggersContentsL, "ControlllerEditTriggersContentsL");
    GTKHUOBJECT (ControllerEdit, ControllerEditTriggersContentsR, "ControllerEditTriggersContentsR");
    GTKHUOBJECT (ControllerEdit, ControllerEditTriggersFrameLabel, "ControllerEditTriggersFrameLabel");
    GTKHUOBJECT (ControllerEdit, ControllerEditCurrentControllerCombobox, "ControllerEditCurrentControllerCombobox");
    GTKHUOBJECT (ControllerEdit, ControllerEditSensitivityFrame, "ControllerEditSensitivityFrame");
    GTKHUOBJECT (ControllerEdit, ControllerEditSensitivityFrameAlignment, "ControllerEditSensitivityFrameAlignment");
    GTKHUOBJECT (ControllerEdit, ControllerEditSensitivityFrameTable, "ControllerEditSensitivityFrameTable");
    GTKHUOBJECT (ControllerEdit, ControllerEditSensitivityMinLabel, "ControllerEditSensitivityMinLabel");
    GTKHUOBJECT (ControllerEdit, ControllerEditSensitivityMaxLabel, "ControllerEditSensitivityMaxLabel");
    GTKHUOBJECT (ControllerEdit, ControllerEditSensitivityMaxSpinbox, "ControllerEditSensitivityMaxSpinbox");
    GTKHUOBJECT (ControllerEdit, ControllerEditSensitivityMinSpinbox, "ControllerEditSensitivityMinSpinbox");
    GTKHUOBJECT (ControllerEdit, ControllerEditSensitivityFrameLabel, "ControllerEditSensitivityFrameLabel");
    GTKHUOBJECT (ControllerEdit, ControllerEditPropertiesFrame, "ControllerEditPropertiesFrame");
    GTKHUOBJECT (ControllerEdit, ControllerEditPropertiesFrameAlignment, "ControllerEditPropertiesFrameAlignment");
    GTKHUOBJECT (ControllerEdit, ControllerEditProperties, "ControllerEditProperties");
    GTKHUOBJECT (ControllerEdit, ControllerEditPropertiesActiveCheckbox, "ControllerEditPropertiesActiveCheckbox");
    GTKHUOBJECT (ControllerEdit, ControllerEditPropertiesMemPakCheckbox, "ControllerEditPropertiesMemPakCheckbox");
    GTKHUOBJECT (ControllerEdit, ControllerEditPropertiesFrameLabel, "ControllerEditPropertiesFrameLabel");

    return ControllerEdit;
}

GtkWidget* create_MacroEdit (void)
{
    GtkWidget *MacroEdit;
    GtkWidget *MacroEditHbox;
    GtkWidget *MacroEditHoldFrame;
    GtkWidget *MacroEditHoldFrameAlignment;
    GtkWidget *MacroEditHoldFrameHBox;
    GtkWidget *MacroEditHoldFrameLeft;
    GtkWidget *MacroEditHoldButtonsFrame;
    GtkWidget *MacroEditHoldButtonsFrameAlignment;
    GtkWidget *MacroEditHoldButtonsFrameVBox;
    GtkWidget *MacroEditHoldButtonsA;
    GtkWidget *MacroEditHoldButtonsB;
    GtkWidget *MacroEditHoldButtonsS;
    GtkWidget *MacroEditHoldButtonsFrameLabel;
    GtkWidget *MacroEditHoldTriggersFrame;
    GtkWidget *MacroEditHoldTriggersFrameAlignment;
    GtkWidget *MacroEditHoldTriggersFrameVBox;
    GtkWidget *MacroEditHoldTriggersZ;
    GtkWidget *MacroEditHoldTriggersL;
    GtkWidget *MacroEditHoldTriggersR;
    GtkWidget *MacroEditHoldTriggersFrameLabel;
    GtkWidget *MacroEditHoldFrameRight;
    GtkWidget *MacroEditHoldAnalogFrame;
    GtkWidget *MacroEditHoldAnalogFrameAlignment;
    GtkWidget *MacroEditHoldAnalogFrameTable;
    GtkWidget *MacroEditHoldAnalogU;
    GtkWidget *MacroEditHoldAnalogD;
    GtkWidget *MacroEditHoldAnalogL;
    GtkWidget *MacroEditHoldAnalogR;
    GtkWidget *MacroEditHoldAnalogFrameLabel;
    GtkWidget *MacroEditHoldCButtonsFrame;
    GtkWidget *MacroEditHoldCButtonsFrameAlignment;
    GtkWidget *MacroEditHoldCButtonsFrameTable;
    GtkWidget *MacroEditHoldCButtonsU;
    GtkWidget *MacroEditHoldCButtonsL;
    GtkWidget *MacroEditHoldCButtonsD;
    GtkWidget *MacroEditHoldCButtonsR;
    GtkWidget *MacroEditHoldCButtonsFrameLabel;
    GtkWidget *MacroEditHoldDPadFrame;
    GtkWidget *MacroEditHoldDPadFrameAlignment;
    GtkWidget *MacroEditHoldDPadFrameTable;
    GtkWidget *MacroEditHoldDPadU;
    GtkWidget *MacroEditHoldDPadL;
    GtkWidget *MacroEditHoldDPadD;
    GtkWidget *MacroEditHoldDPadR;
    GtkWidget *MacroEditHoldDPadFrameLabel;
    GtkWidget *MacroEditHoldFrameLabel;
    GtkWidget *MacroEditComboVBox;
    GtkWidget *MacroEditComboFrame;
    GtkWidget *MacroEditComboFrameAlignment;
    GtkWidget *MacroEditComboFrameVBox;
    GtkWidget *MacroEditComboPlaysCombo;
    GtkWidget *MacroEditComboListHBox;
    GtkWidget *MacroEditComboListWindow;
    GtkWidget *MacroEditComboListTreeView;
    GtkWidget *MacroEditComboListButtons;
    GtkWidget *MacroEditComboListAdd;
    GtkWidget *MacroEditComboListRemove;
    GtkWidget *MacroEditComboListEdit;
    GtkWidget *MacroEditComboModeLabel;
    GtkWidget *MacroEditComboModeVBox;
    GtkWidget *MacroEditComboModeSingleCycle;
    GSList *MacroEditComboModeSingleCycle_group = NULL;
    GtkWidget *MacroEditComboModeContinuousCycleVBox;
    GtkWidget *MacroEditComboModeContinuousCycle;
    GtkWidget *MacroEditComboModeContinuousCycleFrameIntervalHBox;
    GtkWidget *MacroEditComboModeContinuousCycleFrameIntervalVBox;
    GtkWidget *MacroEditComboModeContinuousCycleFrameIntervalLabel;
    GtkObject *MacroEditComboModeContinuousCycleFrameIntervalSpinbox_adj;
    GtkWidget *MacroEditComboModeContinuousCycleFrameIntervalSpinbox;
    GtkWidget *MacroEditComboModeSticky;
    GtkWidget *MacroEditComboFrameLabel;
    GtkWidget *MacroEditButtons;
    GtkWidget *MacroEditCancel;
    GtkWidget *MacroEditOK;

    MacroEdit = gtk_window_new (GTK_WINDOW_POPUP);
    gtk_window_set_title (GTK_WINDOW (MacroEdit), tr("Macro"));
    gtk_window_set_position (GTK_WINDOW (MacroEdit), GTK_WIN_POS_CENTER_ON_PARENT);

    MacroEditHbox = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (MacroEditHbox);
    gtk_container_add (GTK_CONTAINER (MacroEdit), MacroEditHbox);

    MacroEditHoldFrame = gtk_frame_new (NULL);
    gtk_widget_show (MacroEditHoldFrame);
    gtk_box_pack_start (GTK_BOX (MacroEditHbox), MacroEditHoldFrame, FALSE, TRUE, 0);
    gtk_widget_set_size_request (MacroEditHoldFrame, 195, -1);
    gtk_container_set_border_width (GTK_CONTAINER (MacroEditHoldFrame), 15);

    MacroEditHoldFrameAlignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (MacroEditHoldFrameAlignment);
    gtk_container_add (GTK_CONTAINER (MacroEditHoldFrame), MacroEditHoldFrameAlignment);
    gtk_alignment_set_padding (GTK_ALIGNMENT (MacroEditHoldFrameAlignment), 0, 6, 6, 6);

    MacroEditHoldFrameHBox = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (MacroEditHoldFrameHBox);
    gtk_container_add (GTK_CONTAINER (MacroEditHoldFrameAlignment), MacroEditHoldFrameHBox);

    MacroEditHoldFrameLeft = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (MacroEditHoldFrameLeft);
    gtk_box_pack_start (GTK_BOX (MacroEditHoldFrameHBox), MacroEditHoldFrameLeft, TRUE, TRUE, 0);

    MacroEditHoldButtonsFrame = gtk_frame_new (NULL);
    gtk_widget_show (MacroEditHoldButtonsFrame);
    gtk_box_pack_start (GTK_BOX (MacroEditHoldFrameLeft), MacroEditHoldButtonsFrame, TRUE, TRUE, 0);

    MacroEditHoldButtonsFrameAlignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (MacroEditHoldButtonsFrameAlignment);
    gtk_container_add (GTK_CONTAINER (MacroEditHoldButtonsFrame), MacroEditHoldButtonsFrameAlignment);
    gtk_alignment_set_padding (GTK_ALIGNMENT (MacroEditHoldButtonsFrameAlignment), 0, 6, 6, 6);

    MacroEditHoldButtonsFrameVBox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (MacroEditHoldButtonsFrameVBox);
    gtk_container_add (GTK_CONTAINER (MacroEditHoldButtonsFrameAlignment), MacroEditHoldButtonsFrameVBox);

    MacroEditHoldButtonsA = gtk_check_button_new_with_mnemonic (tr("A"));
    gtk_widget_show (MacroEditHoldButtonsA);
    gtk_box_pack_start (GTK_BOX (MacroEditHoldButtonsFrameVBox), MacroEditHoldButtonsA, TRUE, TRUE, 0);

    MacroEditHoldButtonsB = gtk_check_button_new_with_mnemonic (tr("B"));
    gtk_widget_show (MacroEditHoldButtonsB);
    gtk_box_pack_start (GTK_BOX (MacroEditHoldButtonsFrameVBox), MacroEditHoldButtonsB, TRUE, TRUE, 0);

    MacroEditHoldButtonsS = gtk_check_button_new_with_mnemonic (tr("S"));
    gtk_widget_show (MacroEditHoldButtonsS);
    gtk_box_pack_start (GTK_BOX (MacroEditHoldButtonsFrameVBox), MacroEditHoldButtonsS, TRUE, TRUE, 0);

    MacroEditHoldButtonsFrameLabel = gtk_label_new (tr("Buttons"));
    gtk_widget_show (MacroEditHoldButtonsFrameLabel);
    gtk_frame_set_label_widget (GTK_FRAME (MacroEditHoldButtonsFrame), MacroEditHoldButtonsFrameLabel);
    gtk_label_set_use_markup (GTK_LABEL (MacroEditHoldButtonsFrameLabel), TRUE);

    MacroEditHoldTriggersFrame = gtk_frame_new (NULL);
    gtk_widget_show (MacroEditHoldTriggersFrame);
    gtk_box_pack_start (GTK_BOX (MacroEditHoldFrameLeft), MacroEditHoldTriggersFrame, TRUE, TRUE, 0);

    MacroEditHoldTriggersFrameAlignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (MacroEditHoldTriggersFrameAlignment);
    gtk_container_add (GTK_CONTAINER (MacroEditHoldTriggersFrame), MacroEditHoldTriggersFrameAlignment);
    gtk_alignment_set_padding (GTK_ALIGNMENT (MacroEditHoldTriggersFrameAlignment), 0, 6, 6, 6);

    MacroEditHoldTriggersFrameVBox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (MacroEditHoldTriggersFrameVBox);
    gtk_container_add (GTK_CONTAINER (MacroEditHoldTriggersFrameAlignment), MacroEditHoldTriggersFrameVBox);

    MacroEditHoldTriggersZ = gtk_check_button_new_with_mnemonic (tr("Z"));
    gtk_widget_show (MacroEditHoldTriggersZ);
    gtk_box_pack_start (GTK_BOX (MacroEditHoldTriggersFrameVBox), MacroEditHoldTriggersZ, TRUE, TRUE, 0);

    MacroEditHoldTriggersL = gtk_check_button_new_with_mnemonic (tr("L"));
    gtk_widget_show (MacroEditHoldTriggersL);
    gtk_box_pack_start (GTK_BOX (MacroEditHoldTriggersFrameVBox), MacroEditHoldTriggersL, TRUE, TRUE, 0);

    MacroEditHoldTriggersR = gtk_check_button_new_with_mnemonic (tr("R"));
    gtk_widget_show (MacroEditHoldTriggersR);
    gtk_box_pack_start (GTK_BOX (MacroEditHoldTriggersFrameVBox), MacroEditHoldTriggersR, TRUE, TRUE, 0);

    MacroEditHoldTriggersFrameLabel = gtk_label_new (tr("Triggers"));
    gtk_widget_show (MacroEditHoldTriggersFrameLabel);
    gtk_frame_set_label_widget (GTK_FRAME (MacroEditHoldTriggersFrame), MacroEditHoldTriggersFrameLabel);
    gtk_label_set_use_markup (GTK_LABEL (MacroEditHoldTriggersFrameLabel), TRUE);

    MacroEditHoldFrameRight = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (MacroEditHoldFrameRight);
    gtk_box_pack_start (GTK_BOX (MacroEditHoldFrameHBox), MacroEditHoldFrameRight, TRUE, TRUE, 0);

    MacroEditHoldAnalogFrame = gtk_frame_new (NULL);
    gtk_widget_show (MacroEditHoldAnalogFrame);
    gtk_box_pack_start (GTK_BOX (MacroEditHoldFrameRight), MacroEditHoldAnalogFrame, TRUE, TRUE, 0);

    MacroEditHoldAnalogFrameAlignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (MacroEditHoldAnalogFrameAlignment);
    gtk_container_add (GTK_CONTAINER (MacroEditHoldAnalogFrame), MacroEditHoldAnalogFrameAlignment);
    gtk_alignment_set_padding (GTK_ALIGNMENT (MacroEditHoldAnalogFrameAlignment), 0, 6, 6, 6);

    MacroEditHoldAnalogFrameTable = gtk_table_new (2, 2, FALSE);
    gtk_widget_show (MacroEditHoldAnalogFrameTable);
    gtk_container_add (GTK_CONTAINER (MacroEditHoldAnalogFrameAlignment), MacroEditHoldAnalogFrameTable);

    MacroEditHoldAnalogU = gtk_check_button_new_with_mnemonic (tr("U"));
    gtk_widget_show (MacroEditHoldAnalogU);
    gtk_table_attach (GTK_TABLE (MacroEditHoldAnalogFrameTable), MacroEditHoldAnalogU, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);

    MacroEditHoldAnalogD = gtk_check_button_new_with_mnemonic (tr("D"));
    gtk_widget_show (MacroEditHoldAnalogD);
    gtk_table_attach (GTK_TABLE (MacroEditHoldAnalogFrameTable), MacroEditHoldAnalogD, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);

    MacroEditHoldAnalogL = gtk_check_button_new_with_mnemonic (tr("L"));
    gtk_widget_show (MacroEditHoldAnalogL);
    gtk_table_attach (GTK_TABLE (MacroEditHoldAnalogFrameTable), MacroEditHoldAnalogL, 1, 2, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);

    MacroEditHoldAnalogR = gtk_check_button_new_with_mnemonic (tr("R"));
    gtk_widget_show (MacroEditHoldAnalogR);
    gtk_table_attach (GTK_TABLE (MacroEditHoldAnalogFrameTable), MacroEditHoldAnalogR, 1, 2, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);

    MacroEditHoldAnalogFrameLabel = gtk_label_new (tr("Analog Stick"));
    gtk_widget_show (MacroEditHoldAnalogFrameLabel);
    gtk_frame_set_label_widget (GTK_FRAME (MacroEditHoldAnalogFrame), MacroEditHoldAnalogFrameLabel);
    gtk_label_set_use_markup (GTK_LABEL (MacroEditHoldAnalogFrameLabel), TRUE);

    MacroEditHoldCButtonsFrame = gtk_frame_new (NULL);
    gtk_widget_show (MacroEditHoldCButtonsFrame);
    gtk_box_pack_start (GTK_BOX (MacroEditHoldFrameRight), MacroEditHoldCButtonsFrame, TRUE, TRUE, 0);

    MacroEditHoldCButtonsFrameAlignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (MacroEditHoldCButtonsFrameAlignment);
    gtk_container_add (GTK_CONTAINER (MacroEditHoldCButtonsFrame), MacroEditHoldCButtonsFrameAlignment);
    gtk_alignment_set_padding (GTK_ALIGNMENT (MacroEditHoldCButtonsFrameAlignment), 0, 6, 6, 6);

    MacroEditHoldAnalogFrameTable = gtk_table_new (2, 2, FALSE);
    gtk_widget_show (MacroEditHoldAnalogFrameTable);
    gtk_container_add (GTK_CONTAINER (MacroEditHoldCButtonsFrameAlignment), MacroEditHoldAnalogFrameTable);

    MacroEditHoldAnalogU = gtk_check_button_new_with_mnemonic (tr("U"));
    gtk_widget_show (MacroEditHoldAnalogU);
    gtk_table_attach (GTK_TABLE (MacroEditHoldAnalogFrameTable), MacroEditHoldAnalogU, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);

    MacroEditHoldAnalogL = gtk_check_button_new_with_mnemonic (tr("L"));
    gtk_widget_show (MacroEditHoldAnalogL);
    gtk_table_attach (GTK_TABLE (MacroEditHoldAnalogFrameTable), MacroEditHoldAnalogL, 1, 2, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);

    MacroEditHoldAnalogD = gtk_check_button_new_with_mnemonic (tr("D"));
    gtk_widget_show (MacroEditHoldAnalogD);
    gtk_table_attach (GTK_TABLE (MacroEditHoldAnalogFrameTable), MacroEditHoldAnalogD, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);

    MacroEditHoldAnalogR = gtk_check_button_new_with_mnemonic (tr("R"));
    gtk_widget_show (MacroEditHoldAnalogR);
    gtk_table_attach (GTK_TABLE (MacroEditHoldAnalogFrameTable), MacroEditHoldAnalogR, 1, 2, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);

    MacroEditHoldAnalogFrameLabel = gtk_label_new (tr("C Buttons"));
    gtk_widget_show (MacroEditHoldAnalogFrameLabel);
    gtk_frame_set_label_widget (GTK_FRAME (MacroEditHoldCButtonsFrame), MacroEditHoldAnalogFrameLabel);
    gtk_label_set_use_markup (GTK_LABEL (MacroEditHoldAnalogFrameLabel), TRUE);

    MacroEditHoldDPadFrame = gtk_frame_new (NULL);
    gtk_widget_show (MacroEditHoldDPadFrame);
    gtk_box_pack_start (GTK_BOX (MacroEditHoldFrameRight), MacroEditHoldDPadFrame, TRUE, TRUE, 0);

    MacroEditHoldDPadFrameAlignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (MacroEditHoldDPadFrameAlignment);
    gtk_container_add (GTK_CONTAINER (MacroEditHoldDPadFrame), MacroEditHoldDPadFrameAlignment);
    gtk_alignment_set_padding (GTK_ALIGNMENT (MacroEditHoldDPadFrameAlignment), 0, 6, 6, 6);

    MacroEditHoldDPadFrameTable = gtk_table_new (2, 2, FALSE);
    gtk_widget_show (MacroEditHoldDPadFrameTable);
    gtk_container_add (GTK_CONTAINER (MacroEditHoldDPadFrameAlignment), MacroEditHoldDPadFrameTable);

    MacroEditHoldDPadU = gtk_check_button_new_with_mnemonic (tr("U"));
    gtk_widget_show (MacroEditHoldDPadU);
    gtk_table_attach (GTK_TABLE (MacroEditHoldDPadFrameTable), MacroEditHoldDPadU, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);

    MacroEditHoldDPadL = gtk_check_button_new_with_mnemonic (tr("L"));
    gtk_widget_show (MacroEditHoldDPadL);
    gtk_table_attach (GTK_TABLE (MacroEditHoldDPadFrameTable), MacroEditHoldDPadL, 1, 2, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);

    MacroEditHoldDPadD = gtk_check_button_new_with_mnemonic (tr("D"));
    gtk_widget_show (MacroEditHoldDPadD);
    gtk_table_attach (GTK_TABLE (MacroEditHoldDPadFrameTable), MacroEditHoldDPadD, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);

    MacroEditHoldDPadR = gtk_check_button_new_with_mnemonic (tr("R"));
    gtk_widget_show (MacroEditHoldDPadR);
    gtk_table_attach (GTK_TABLE (MacroEditHoldDPadFrameTable), MacroEditHoldDPadR, 1, 2, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                       (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);

    MacroEditHoldDPadFrameLabel = gtk_label_new (tr("D Pad"));
    gtk_widget_show (MacroEditHoldDPadFrameLabel);
    gtk_frame_set_label_widget (GTK_FRAME (MacroEditHoldDPadFrame), MacroEditHoldDPadFrameLabel);
    gtk_label_set_use_markup (GTK_LABEL (MacroEditHoldDPadFrameLabel), TRUE);

    MacroEditHoldFrameLabel = gtk_label_new (tr("Hold"));
    gtk_widget_show (MacroEditHoldFrameLabel);
    gtk_frame_set_label_widget (GTK_FRAME (MacroEditHoldFrame), MacroEditHoldFrameLabel);
    gtk_label_set_use_markup (GTK_LABEL (MacroEditHoldFrameLabel), TRUE);

    MacroEditComboVBox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (MacroEditComboVBox);
    gtk_box_pack_start (GTK_BOX (MacroEditHbox), MacroEditComboVBox, TRUE, TRUE, 0);

    MacroEditComboFrame = gtk_frame_new (NULL);
    gtk_widget_show (MacroEditComboFrame);
    gtk_box_pack_start (GTK_BOX (MacroEditComboVBox), MacroEditComboFrame, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (MacroEditComboFrame), 15);

    MacroEditComboFrameAlignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (MacroEditComboFrameAlignment);
    gtk_container_add (GTK_CONTAINER (MacroEditComboFrame), MacroEditComboFrameAlignment);
    gtk_alignment_set_padding (GTK_ALIGNMENT (MacroEditComboFrameAlignment), 0, 6, 6, 6);

    MacroEditComboFrameVBox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (MacroEditComboFrameVBox);
    gtk_container_add (GTK_CONTAINER (MacroEditComboFrameAlignment), MacroEditComboFrameVBox);

    MacroEditComboPlaysCombo = gtk_check_button_new_with_mnemonic (tr("Plays Combo"));
    gtk_widget_show (MacroEditComboPlaysCombo);
    gtk_box_pack_start (GTK_BOX (MacroEditComboFrameVBox), MacroEditComboPlaysCombo, FALSE, FALSE, 0);

    MacroEditComboListHBox = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (MacroEditComboListHBox);
    gtk_box_pack_start (GTK_BOX (MacroEditComboFrameVBox), MacroEditComboListHBox, FALSE, TRUE, 0);

    MacroEditComboListWindow = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_show (MacroEditComboListWindow);
    gtk_box_pack_start (GTK_BOX (MacroEditComboListHBox), MacroEditComboListWindow, TRUE, TRUE, 0);
    gtk_widget_set_size_request (MacroEditComboListWindow, 132, -1);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (MacroEditComboListWindow), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (MacroEditComboListWindow), GTK_SHADOW_IN);

    MacroEditComboListTreeView = gtk_tree_view_new ();
    gtk_widget_show (MacroEditComboListTreeView);
    gtk_container_add (GTK_CONTAINER (MacroEditComboListWindow), MacroEditComboListTreeView);
    gtk_tree_view_set_reorderable (GTK_TREE_VIEW (MacroEditComboListTreeView), TRUE);

    MacroEditComboListButtons = gtk_vbutton_box_new ();
    gtk_widget_show (MacroEditComboListButtons);
    gtk_box_pack_start (GTK_BOX (MacroEditComboListHBox), MacroEditComboListButtons, TRUE, TRUE, 0);

    MacroEditComboListAdd = gtk_button_new_from_stock ("gtk-add");
    gtk_widget_show (MacroEditComboListAdd);
    gtk_container_add (GTK_CONTAINER (MacroEditComboListButtons), MacroEditComboListAdd);
    GTK_WIDGET_SET_FLAGS (MacroEditComboListAdd, GTK_CAN_DEFAULT);

    MacroEditComboListRemove = gtk_button_new_from_stock ("gtk-remove");
    gtk_widget_show (MacroEditComboListRemove);
    gtk_container_add (GTK_CONTAINER (MacroEditComboListButtons), MacroEditComboListRemove);
    GTK_WIDGET_SET_FLAGS (MacroEditComboListRemove, GTK_CAN_DEFAULT);

    MacroEditComboListEdit = gtk_button_new_from_stock ("gtk-edit");
    gtk_widget_show (MacroEditComboListEdit);
    gtk_container_add (GTK_CONTAINER (MacroEditComboListButtons), MacroEditComboListEdit);
    GTK_WIDGET_SET_FLAGS (MacroEditComboListEdit, GTK_CAN_DEFAULT);

    MacroEditComboModeLabel = gtk_label_new (tr("Mode"));
    gtk_widget_show (MacroEditComboModeLabel);
    gtk_box_pack_start (GTK_BOX (MacroEditComboFrameVBox), MacroEditComboModeLabel, FALSE, FALSE, 0);

    MacroEditComboModeVBox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (MacroEditComboModeVBox);
    gtk_box_pack_start (GTK_BOX (MacroEditComboFrameVBox), MacroEditComboModeVBox, TRUE, TRUE, 0);

    MacroEditComboModeSingleCycle = gtk_radio_button_new_with_mnemonic (NULL, tr("Single Cycle"));
    gtk_widget_show (MacroEditComboModeSingleCycle);
    gtk_box_pack_start (GTK_BOX (MacroEditComboModeVBox), MacroEditComboModeSingleCycle, FALSE, FALSE, 0);
    gtk_radio_button_set_group (GTK_RADIO_BUTTON (MacroEditComboModeSingleCycle), MacroEditComboModeSingleCycle_group);
    MacroEditComboModeSingleCycle_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (MacroEditComboModeSingleCycle));

    MacroEditComboModeContinuousCycleVBox = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (MacroEditComboModeContinuousCycleVBox);
    gtk_box_pack_start (GTK_BOX (MacroEditComboModeVBox), MacroEditComboModeContinuousCycleVBox, FALSE, FALSE, 0);

    MacroEditComboModeContinuousCycle = gtk_radio_button_new_with_mnemonic (NULL, tr("Continuous Cycle"));
    gtk_widget_show (MacroEditComboModeContinuousCycle);
    gtk_box_pack_start (GTK_BOX (MacroEditComboModeContinuousCycleVBox), MacroEditComboModeContinuousCycle, FALSE, FALSE, 0);
    gtk_radio_button_set_group (GTK_RADIO_BUTTON (MacroEditComboModeContinuousCycle), MacroEditComboModeSingleCycle_group);
    MacroEditComboModeSingleCycle_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (MacroEditComboModeContinuousCycle));

    MacroEditComboModeContinuousCycleFrameIntervalHBox = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (MacroEditComboModeContinuousCycleFrameIntervalHBox);
    gtk_box_pack_start (GTK_BOX (MacroEditComboModeContinuousCycleVBox), MacroEditComboModeContinuousCycleFrameIntervalHBox, TRUE, TRUE, 0);

    MacroEditComboModeContinuousCycleFrameIntervalVBox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (MacroEditComboModeContinuousCycleFrameIntervalVBox);
    gtk_box_pack_start (GTK_BOX (MacroEditComboModeContinuousCycleFrameIntervalHBox), MacroEditComboModeContinuousCycleFrameIntervalVBox, TRUE, TRUE, 0);

    MacroEditComboModeContinuousCycleFrameIntervalLabel = gtk_label_new (tr("Frame Interval"));
    gtk_widget_show (MacroEditComboModeContinuousCycleFrameIntervalLabel);
    gtk_box_pack_start (GTK_BOX (MacroEditComboModeContinuousCycleFrameIntervalVBox), MacroEditComboModeContinuousCycleFrameIntervalLabel, FALSE, FALSE, 0);

    MacroEditComboModeContinuousCycleFrameIntervalSpinbox_adj = gtk_adjustment_new (1, 1, 1000, 1, 10, 10);
    MacroEditComboModeContinuousCycleFrameIntervalSpinbox = gtk_spin_button_new (GTK_ADJUSTMENT (MacroEditComboModeContinuousCycleFrameIntervalSpinbox_adj), 1, 0);
    gtk_widget_show (MacroEditComboModeContinuousCycleFrameIntervalSpinbox);
    gtk_box_pack_start (GTK_BOX (MacroEditComboModeContinuousCycleFrameIntervalVBox), MacroEditComboModeContinuousCycleFrameIntervalSpinbox, FALSE, FALSE, 0);
    gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (MacroEditComboModeContinuousCycleFrameIntervalSpinbox), TRUE);
    gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (MacroEditComboModeContinuousCycleFrameIntervalSpinbox), GTK_UPDATE_IF_VALID);
    gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (MacroEditComboModeContinuousCycleFrameIntervalSpinbox), TRUE);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (MacroEditComboModeContinuousCycleFrameIntervalSpinbox), TRUE);

    MacroEditComboModeSticky = gtk_radio_button_new_with_mnemonic (NULL, tr("Sticky"));
    gtk_widget_show (MacroEditComboModeSticky);
    gtk_box_pack_start (GTK_BOX (MacroEditComboModeVBox), MacroEditComboModeSticky, FALSE, FALSE, 0);
    gtk_radio_button_set_group (GTK_RADIO_BUTTON (MacroEditComboModeSticky), MacroEditComboModeSingleCycle_group);
    MacroEditComboModeSingleCycle_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (MacroEditComboModeSticky));

    MacroEditComboFrameLabel = gtk_label_new (tr("Combo"));
    gtk_widget_show (MacroEditComboFrameLabel);
    gtk_frame_set_label_widget (GTK_FRAME (MacroEditComboFrame), MacroEditComboFrameLabel);
    gtk_label_set_use_markup (GTK_LABEL (MacroEditComboFrameLabel), TRUE);

    MacroEditButtons = gtk_hbutton_box_new ();
    gtk_widget_show (MacroEditButtons);
    gtk_box_pack_start (GTK_BOX (MacroEditComboVBox), MacroEditButtons, FALSE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (MacroEditButtons), 15);
    gtk_button_box_set_layout (GTK_BUTTON_BOX (MacroEditButtons), GTK_BUTTONBOX_END);

    MacroEditCancel = gtk_button_new_from_stock ("gtk-cancel");
    gtk_widget_show (MacroEditCancel);
    gtk_container_add (GTK_CONTAINER (MacroEditButtons), MacroEditCancel);
    GTK_WIDGET_SET_FLAGS (MacroEditCancel, GTK_CAN_DEFAULT);

    MacroEditOK = gtk_button_new_from_stock ("gtk-ok");
    gtk_widget_show (MacroEditOK);
    gtk_container_add (GTK_CONTAINER (MacroEditButtons), MacroEditOK);
    GTK_WIDGET_SET_FLAGS (MacroEditOK, GTK_CAN_DEFAULT);

    /* Store pointers to all widgets, for use by lookup_widget(). */
    GTKHUOBJECT_NO_REF (MacroEdit, MacroEdit, "MacroEdit");
    GTKHUOBJECT (MacroEdit, MacroEditHbox, "MacroEditHbox");
    GTKHUOBJECT (MacroEdit, MacroEditHoldFrame, "MacroEditHoldFrame");
    GTKHUOBJECT (MacroEdit, MacroEditHoldFrameAlignment, "MacroEditHoldFrameAlignment");
    GTKHUOBJECT (MacroEdit, MacroEditHoldFrameHBox, "MacroEditHoldFrameHBox");
    GTKHUOBJECT (MacroEdit, MacroEditHoldFrameLeft, "MacroEditHoldFrameLeft");
    GTKHUOBJECT (MacroEdit, MacroEditHoldButtonsFrame, "MacroEditHoldButtonsFrame");
    GTKHUOBJECT (MacroEdit, MacroEditHoldButtonsFrameAlignment, "MacroEditHoldButtonsFrameAlignment");
    GTKHUOBJECT (MacroEdit, MacroEditHoldButtonsFrameVBox, "MacroEditHoldButtonsFrameVBox");
    GTKHUOBJECT (MacroEdit, MacroEditHoldButtonsA, "MacroEditHoldButtonsA");
    GTKHUOBJECT (MacroEdit, MacroEditHoldButtonsB, "MacroEditHoldButtonsB");
    GTKHUOBJECT (MacroEdit, MacroEditHoldButtonsS, "MacroEditHoldButtonsS");
    GTKHUOBJECT (MacroEdit, MacroEditHoldButtonsFrameLabel, "MacroEditHoldButtonsFrameLabel");
    GTKHUOBJECT (MacroEdit, MacroEditHoldTriggersFrame, "MacroEditHoldTriggersFrame");
    GTKHUOBJECT (MacroEdit, MacroEditHoldTriggersFrameAlignment, "MacroEditHoldTriggersFrameAlignment");
    GTKHUOBJECT (MacroEdit, MacroEditHoldTriggersFrameVBox, "MacroEditHoldTriggersFrameVBox");
    GTKHUOBJECT (MacroEdit, MacroEditHoldTriggersZ, "MacroEditHoldTriggersZ");
    GTKHUOBJECT (MacroEdit, MacroEditHoldTriggersL, "MacroEditHoldTriggersL");
    GTKHUOBJECT (MacroEdit, MacroEditHoldTriggersR, "MacroEditHoldTriggersR");
    GTKHUOBJECT (MacroEdit, MacroEditHoldTriggersFrameLabel, "MacroEditHoldTriggersFrameLabel");
    GTKHUOBJECT (MacroEdit, MacroEditHoldFrameRight, "MacroEditHoldFrameRight");
    GTKHUOBJECT (MacroEdit, MacroEditHoldAnalogFrame, "MacroEditHoldAnalogFrame");
    GTKHUOBJECT (MacroEdit, MacroEditHoldAnalogFrameAlignment, "MacroEditHoldAnalogFrameAlignment");
    GTKHUOBJECT (MacroEdit, MacroEditHoldAnalogFrameTable, "MacroEditHoldAnalogFrameTable");
    GTKHUOBJECT (MacroEdit, MacroEditHoldAnalogU, "MacroEditHoldAnalogU");
    GTKHUOBJECT (MacroEdit, MacroEditHoldAnalogD, "MacroEditHoldAnalogD");
    GTKHUOBJECT (MacroEdit, MacroEditHoldAnalogL, "MacroEditHoldAnalogL");
    GTKHUOBJECT (MacroEdit, MacroEditHoldAnalogR, "MacroEditHoldAnalogR");
    GTKHUOBJECT (MacroEdit, MacroEditHoldAnalogFrameLabel, "MacroEditHoldAnalogFrameLabel");
    GTKHUOBJECT (MacroEdit, MacroEditHoldCButtonsFrame, "MacroEditHoldCButtonsFrame");
    GTKHUOBJECT (MacroEdit, MacroEditHoldCButtonsFrameAlignment, "MacroEditHoldCButtonsFrameAlignment");
    GTKHUOBJECT (MacroEdit, MacroEditHoldAnalogFrameTable, "MacroEditHoldAnalogFrameTable");
    GTKHUOBJECT (MacroEdit, MacroEditHoldAnalogU, "MacroEditHoldAnalogU");
    GTKHUOBJECT (MacroEdit, MacroEditHoldAnalogL, "MacroEditHoldAnalogL");
    GTKHUOBJECT (MacroEdit, MacroEditHoldAnalogD, "MacroEditHoldAnalogD");
    GTKHUOBJECT (MacroEdit, MacroEditHoldAnalogR, "MacroEditHoldAnalogR");
    GTKHUOBJECT (MacroEdit, MacroEditHoldAnalogFrameLabel, "MacroEditHoldAnalogFrameLabel");
    GTKHUOBJECT (MacroEdit, MacroEditHoldDPadFrame, "MacroEditHoldDPadFrame");
    GTKHUOBJECT (MacroEdit, MacroEditHoldDPadFrameAlignment, "MacroEditHoldDPadFrameAlignment");
    GTKHUOBJECT (MacroEdit, MacroEditHoldDPadFrameTable, "MacroEditHoldDPadFrameTable");
    GTKHUOBJECT (MacroEdit, MacroEditHoldDPadU, "MacroEditHoldDPadU");
    GTKHUOBJECT (MacroEdit, MacroEditHoldDPadL, "MacroEditHoldDPadL");
    GTKHUOBJECT (MacroEdit, MacroEditHoldDPadD, "MacroEditHoldDPadD");
    GTKHUOBJECT (MacroEdit, MacroEditHoldDPadR, "MacroEditHoldDPadR");
    GTKHUOBJECT (MacroEdit, MacroEditHoldDPadFrameLabel, "MacroEditHoldDPadFrameLabel");
    GTKHUOBJECT (MacroEdit, MacroEditHoldFrameLabel, "MacroEditHoldFrameLabel");
    GTKHUOBJECT (MacroEdit, MacroEditComboVBox, "MacroEditComboVBox");
    GTKHUOBJECT (MacroEdit, MacroEditComboFrame, "MacroEditComboFrame");
    GTKHUOBJECT (MacroEdit, MacroEditComboFrameAlignment, "MacroEditComboFrameAlignment");
    GTKHUOBJECT (MacroEdit, MacroEditComboFrameVBox, "MacroEditComboFrameVBox");
    GTKHUOBJECT (MacroEdit, MacroEditComboPlaysCombo, "MacroEditComboPlaysCombo");
    GTKHUOBJECT (MacroEdit, MacroEditComboListHBox, "MacroEditComboListHBox");
    GTKHUOBJECT (MacroEdit, MacroEditComboListWindow, "MacroEditComboListWindow");
    GTKHUOBJECT (MacroEdit, MacroEditComboListTreeView, "MacroEditComboListTreeView");
    GTKHUOBJECT (MacroEdit, MacroEditComboListButtons, "MacroEditComboListButtons");
    GTKHUOBJECT (MacroEdit, MacroEditComboListAdd, "MacroEditComboListAdd");
    GTKHUOBJECT (MacroEdit, MacroEditComboListRemove, "MacroEditComboListRemove");
    GTKHUOBJECT (MacroEdit, MacroEditComboListEdit, "MacroEditComboListEdit");
    GTKHUOBJECT (MacroEdit, MacroEditComboModeLabel, "MacroEditComboModeLabel");
    GTKHUOBJECT (MacroEdit, MacroEditComboModeVBox, "MacroEditComboModeVBox");
    GTKHUOBJECT (MacroEdit, MacroEditComboModeSingleCycle, "MacroEditComboModeSingleCycle");
    GTKHUOBJECT (MacroEdit, MacroEditComboModeContinuousCycleVBox, "MacroEditComboModeContinuousCycleVBox");
    GTKHUOBJECT (MacroEdit, MacroEditComboModeContinuousCycle, "MacroEditComboModeContinuousCycle");
    GTKHUOBJECT (MacroEdit, MacroEditComboModeContinuousCycleFrameIntervalHBox, "MacroEditComboModeContinuousCycleFrameIntervalHBox");
    GTKHUOBJECT (MacroEdit, MacroEditComboModeContinuousCycleFrameIntervalVBox, "MacroEditComboModeContinuousCycleFrameIntervalVBox");
    GTKHUOBJECT (MacroEdit, MacroEditComboModeContinuousCycleFrameIntervalLabel, "MacroEditComboModeContinuousCycleFrameIntervalLabel");
    GTKHUOBJECT (MacroEdit, MacroEditComboModeContinuousCycleFrameIntervalSpinbox, "MacroEditComboModeContinuousCycleFrameIntervalSpinbox");
    GTKHUOBJECT (MacroEdit, MacroEditComboModeSticky, "MacroEditComboModeSticky");
    GTKHUOBJECT (MacroEdit, MacroEditComboFrameLabel, "MacroEditComboFrameLabel");
    GTKHUOBJECT (MacroEdit, MacroEditButtons, "MacroEditButtons");
    GTKHUOBJECT (MacroEdit, MacroEditCancel, "MacroEditCancel");
    GTKHUOBJECT (MacroEdit, MacroEditOK, "MacroEditOK");

    return MacroEdit;
}

GtkWidget* create_EventEdit (void)
{
    GtkWidget *EventEdit;
    GtkWidget *EventEditVBox;
    GtkWidget *EventEditEventVBox;
    GtkWidget *EventEditTimerEventFrame;
    GtkWidget *EventEditTimerEventAlingment;
    GtkWidget *EventEditTimerEventVBox;
    GtkWidget *EventEditTimerEventUse;
    GSList *EventEditTimerEventUse_group = NULL;
    GtkWidget *EventEditTimerPauseVBox;
    GtkWidget *EventEditTimerPauseLabel;
    GtkObject *EventEditTimerPauseSpinbox_adj;
    GtkWidget *EventEditTimerPauseSpinbox;
    GtkWidget *EventEditTimerPauseLabel2;
    GtkWidget *EventEditTimerEventFrameLabel;
    GtkWidget *EventEditControllerEventFrame;
    GtkWidget *EventEditControllerEventFrameAlignment;
    GtkWidget *EventEditControllerEventVBox;
    GtkWidget *EventEditControllerEventUse;
    GtkWidget *EventEditControllerEventWindow;
    GtkWidget *EventEditControllerEventTreeView;
    GtkWidget *EventEditControllerEventTypeHBox;
    GtkWidget *EventEditControllerEventPress;
    GSList *EventEditControllerEventPress_group = NULL;
    GtkWidget *EventEditControllerEventRelease;
    GSList *EventEditControllerEventRelease_group = NULL;
    GtkWidget *EventEditControllerEventFrameLabel;
    GtkWidget *EventEditButtons;
    GtkWidget *EventEditCancel;
    GtkWidget *EventEditOK;

    EventEdit = gtk_window_new (GTK_WINDOW_POPUP);
    gtk_window_set_title (GTK_WINDOW (EventEdit), tr("Select Event"));
    gtk_window_set_position (GTK_WINDOW (EventEdit), GTK_WIN_POS_CENTER_ON_PARENT);

    EventEditVBox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (EventEditVBox);
    gtk_container_add (GTK_CONTAINER (EventEdit), EventEditVBox);

    EventEditEventVBox = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (EventEditEventVBox);
    gtk_box_pack_start (GTK_BOX (EventEditVBox), EventEditEventVBox, TRUE, TRUE, 0);

    EventEditTimerEventFrame = gtk_frame_new (NULL);
    gtk_widget_show (EventEditTimerEventFrame);
    gtk_box_pack_start (GTK_BOX (EventEditEventVBox), EventEditTimerEventFrame, TRUE, TRUE, 0);

    EventEditTimerEventAlingment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (EventEditTimerEventAlingment);
    gtk_container_add (GTK_CONTAINER (EventEditTimerEventFrame), EventEditTimerEventAlingment);
    gtk_alignment_set_padding (GTK_ALIGNMENT (EventEditTimerEventAlingment), 0, 6, 6, 6);

    EventEditTimerEventVBox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (EventEditTimerEventVBox);
    gtk_container_add (GTK_CONTAINER (EventEditTimerEventAlingment), EventEditTimerEventVBox);

    EventEditTimerEventUse = gtk_radio_button_new_with_mnemonic (NULL, tr("Use"));
    gtk_widget_show (EventEditTimerEventUse);
    gtk_box_pack_start (GTK_BOX (EventEditTimerEventVBox), EventEditTimerEventUse, FALSE, FALSE, 0);
    gtk_radio_button_set_group (GTK_RADIO_BUTTON (EventEditTimerEventUse), EventEditTimerEventUse_group);
    EventEditTimerEventUse_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (EventEditTimerEventUse));

    EventEditTimerPauseVBox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (EventEditTimerPauseVBox);
    gtk_box_pack_start (GTK_BOX (EventEditTimerEventVBox), EventEditTimerPauseVBox, TRUE, TRUE, 0);

    EventEditTimerPauseLabel = gtk_label_new (tr("Pause for"));
    gtk_widget_show (EventEditTimerPauseLabel);
    gtk_box_pack_start (GTK_BOX (EventEditTimerPauseVBox), EventEditTimerPauseLabel, FALSE, FALSE, 0);

    EventEditTimerPauseSpinbox_adj = gtk_adjustment_new (1, 1, 1000, 1, 10, 10);
    EventEditTimerPauseSpinbox = gtk_spin_button_new (GTK_ADJUSTMENT (EventEditTimerPauseSpinbox_adj), 1, 0);
    gtk_widget_show (EventEditTimerPauseSpinbox);
    gtk_box_pack_start (GTK_BOX (EventEditTimerPauseVBox), EventEditTimerPauseSpinbox, FALSE, FALSE, 0);
    gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (EventEditTimerPauseSpinbox), TRUE);
    gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (EventEditTimerPauseSpinbox), TRUE);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (EventEditTimerPauseSpinbox), TRUE);

    EventEditTimerPauseLabel2 = gtk_label_new (tr("frames"));
    gtk_widget_show (EventEditTimerPauseLabel2);
    gtk_box_pack_start (GTK_BOX (EventEditTimerPauseVBox), EventEditTimerPauseLabel2, FALSE, FALSE, 0);

    EventEditTimerEventFrameLabel = gtk_label_new (tr("Timer Event"));
    gtk_widget_show (EventEditTimerEventFrameLabel);
    gtk_frame_set_label_widget (GTK_FRAME (EventEditTimerEventFrame), EventEditTimerEventFrameLabel);
    gtk_label_set_use_markup (GTK_LABEL (EventEditTimerEventFrameLabel), TRUE);

    EventEditControllerEventFrame = gtk_frame_new (NULL);
    gtk_widget_show (EventEditControllerEventFrame);
    gtk_box_pack_start (GTK_BOX (EventEditEventVBox), EventEditControllerEventFrame, TRUE, TRUE, 0);

    EventEditControllerEventFrameAlignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (EventEditControllerEventFrameAlignment);
    gtk_container_add (GTK_CONTAINER (EventEditControllerEventFrame), EventEditControllerEventFrameAlignment);
    gtk_alignment_set_padding (GTK_ALIGNMENT (EventEditControllerEventFrameAlignment), 0, 6, 6, 6);

    EventEditControllerEventVBox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (EventEditControllerEventVBox);
    gtk_container_add (GTK_CONTAINER (EventEditControllerEventFrameAlignment), EventEditControllerEventVBox);

    EventEditControllerEventUse = gtk_radio_button_new_with_mnemonic (NULL, tr("Use"));
    gtk_widget_show (EventEditControllerEventUse);
    gtk_box_pack_start (GTK_BOX (EventEditControllerEventVBox), EventEditControllerEventUse, FALSE, FALSE, 0);
    gtk_radio_button_set_group (GTK_RADIO_BUTTON (EventEditControllerEventUse), EventEditTimerEventUse_group);
    EventEditTimerEventUse_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (EventEditControllerEventUse));

    EventEditControllerEventWindow = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_show (EventEditControllerEventWindow);
    gtk_box_pack_start (GTK_BOX (EventEditControllerEventVBox), EventEditControllerEventWindow, TRUE, TRUE, 0);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (EventEditControllerEventWindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (EventEditControllerEventWindow), GTK_SHADOW_IN);

    EventEditControllerEventTreeView = gtk_tree_view_new ();
    gtk_widget_show (EventEditControllerEventTreeView);
    gtk_container_add (GTK_CONTAINER (EventEditControllerEventWindow), EventEditControllerEventTreeView);

    EventEditControllerEventTypeHBox = gtk_hbox_new (TRUE, 0);
    gtk_widget_show (EventEditControllerEventTypeHBox);
    gtk_box_pack_start (GTK_BOX (EventEditControllerEventVBox), EventEditControllerEventTypeHBox, FALSE, TRUE, 0);

    EventEditControllerEventPress = gtk_radio_button_new_with_mnemonic (NULL, tr("Press"));
    gtk_widget_show (EventEditControllerEventPress);
    gtk_box_pack_start (GTK_BOX (EventEditControllerEventTypeHBox), EventEditControllerEventPress, FALSE, FALSE, 0);
    gtk_radio_button_set_group (GTK_RADIO_BUTTON (EventEditControllerEventPress), EventEditControllerEventPress_group);
    EventEditControllerEventPress_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (EventEditControllerEventPress));

    EventEditControllerEventRelease = gtk_radio_button_new_with_mnemonic (NULL, tr("Release"));
    gtk_widget_show (EventEditControllerEventRelease);
    gtk_box_pack_start (GTK_BOX (EventEditControllerEventTypeHBox), EventEditControllerEventRelease, FALSE, FALSE, 0);
    gtk_radio_button_set_group (GTK_RADIO_BUTTON (EventEditControllerEventRelease), EventEditControllerEventRelease_group);
    EventEditControllerEventRelease_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (EventEditControllerEventRelease));

    EventEditControllerEventFrameLabel = gtk_label_new (tr("Controller Event"));
    gtk_widget_show (EventEditControllerEventFrameLabel);
    gtk_frame_set_label_widget (GTK_FRAME (EventEditControllerEventFrame), EventEditControllerEventFrameLabel);
    gtk_label_set_use_markup (GTK_LABEL (EventEditControllerEventFrameLabel), TRUE);

    EventEditButtons = gtk_hbutton_box_new ();
    gtk_widget_show (EventEditButtons);
    gtk_box_pack_start (GTK_BOX (EventEditVBox), EventEditButtons, FALSE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (EventEditButtons), 5);
    gtk_button_box_set_layout (GTK_BUTTON_BOX (EventEditButtons), GTK_BUTTONBOX_END);

    EventEditCancel = gtk_button_new_from_stock ("gtk-cancel");
    gtk_widget_show (EventEditCancel);
    gtk_container_add (GTK_CONTAINER (EventEditButtons), EventEditCancel);
    GTK_WIDGET_SET_FLAGS (EventEditCancel, GTK_CAN_DEFAULT);

    EventEditOK = gtk_button_new_from_stock ("gtk-ok");
    gtk_widget_show (EventEditOK);
    gtk_container_add (GTK_CONTAINER (EventEditButtons), EventEditOK);
    GTK_WIDGET_SET_FLAGS (EventEditOK, GTK_CAN_DEFAULT);

    /* Store pointers to all widgets, for use by lookup_widget(). */
    GTKHUOBJECT_NO_REF (EventEdit, EventEdit, "EventEdit");
    GTKHUOBJECT (EventEdit, EventEditVBox, "EventEditVBox");
    GTKHUOBJECT (EventEdit, EventEditEventVBox, "EventEditEventVBox");
    GTKHUOBJECT (EventEdit, EventEditTimerEventFrame, "EventEditTimerEventFrame");
    GTKHUOBJECT (EventEdit, EventEditTimerEventAlingment, "EventEditTimerEventAlingment");
    GTKHUOBJECT (EventEdit, EventEditTimerEventVBox, "EventEditTimerEventVBox");
    GTKHUOBJECT (EventEdit, EventEditTimerEventUse, "EventEditTimerEventUse");
    GTKHUOBJECT (EventEdit, EventEditTimerPauseVBox, "EventEditTimerPauseVBox");
    GTKHUOBJECT (EventEdit, EventEditTimerPauseLabel, "EventEditTimerPauseLabel");
    GTKHUOBJECT (EventEdit, EventEditTimerPauseSpinbox, "EventEditTimerPauseSpinbox");
    GTKHUOBJECT (EventEdit, EventEditTimerPauseLabel2, "EventEditTimerPauseLabel2");
    GTKHUOBJECT (EventEdit, EventEditTimerEventFrameLabel, "EventEditTimerEventFrameLabel");
    GTKHUOBJECT (EventEdit, EventEditControllerEventFrame, "EventEditControllerEventFrame");
    GTKHUOBJECT (EventEdit, EventEditControllerEventFrameAlignment, "EventEditControllerEventFrameAlignment");
    GTKHUOBJECT (EventEdit, EventEditControllerEventVBox, "EventEditControllerEventVBox");
    GTKHUOBJECT (EventEdit, EventEditControllerEventUse, "EventEditControllerEventUse");
    GTKHUOBJECT (EventEdit, EventEditControllerEventWindow, "EventEditControllerEventWindow");
    GTKHUOBJECT (EventEdit, EventEditControllerEventTreeView, "EventEditControllerEventTreeView");
    GTKHUOBJECT (EventEdit, EventEditControllerEventTypeHBox, "EventEditControllerEventTypeHBox");
    GTKHUOBJECT (EventEdit, EventEditControllerEventPress, "EventEditControllerEventPress");
    GTKHUOBJECT (EventEdit, EventEditControllerEventRelease, "EventEditControllerEventRelease");
    GTKHUOBJECT (EventEdit, EventEditControllerEventFrameLabel, "EventEditControllerEventFrameLabel");
    GTKHUOBJECT (EventEdit, EventEditButtons, "EventEditButtons");
    GTKHUOBJECT (EventEdit, EventEditCancel, "EventEditCancel");
    GTKHUOBJECT (EventEdit, EventEditOK, "EventEditOK");

    return EventEdit;
}

bool configure()
{
    GtkWidget *ControllerEdit;
    GtkWidget *MacroEdit;
    GtkWidget *EventEdit;

    ControllerEdit = create_ControllerEdit ();
    gtk_widget_show (ControllerEdit);
    MacroEdit = create_MacroEdit ();
    gtk_widget_show (MacroEdit);
    EventEdit = create_EventEdit ();
    gtk_widget_show (EventEdit);
}

