object ConvDialog: TConvDialog
  Left = 0
  Top = 0
  BorderStyle = bsDialog
  Caption = 'Conversion Option'
  ClientHeight = 111
  ClientWidth = 359
  Color = clWhite
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object Label2: TLabel
    Left = 209
    Top = 11
    Width = 12
    Height = 13
    Caption = 'To'
  end
  object Label10: TLabel
    Left = 10
    Top = 87
    Width = 37
    Height = 13
    Caption = 'Options'
  end
  object Label4: TLabel
    Left = 9
    Top = 34
    Width = 208
    Height = 13
    Caption = 'Message Types  (Inteval: s) separeted by ,'
  end
  object BtnOk: TButton
    Left = 177
    Top = 80
    Width = 88
    Height = 29
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 1
    OnClick = BtnOkClick
  end
  object BtnCancel: TButton
    Left = 265
    Top = 80
    Width = 88
    Height = 29
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 0
  end
  object Conversion: TCheckBox
    Left = 8
    Top = 9
    Width = 106
    Height = 17
    Caption = 'Conversion From'
    TabOrder = 2
    OnClick = ConversionClick
  end
  object InFormat: TComboBox
    Left = 111
    Top = 7
    Width = 93
    Height = 21
    Style = csDropDownList
    DropDownCount = 16
    Enabled = False
    TabOrder = 3
  end
  object OutFormat: TComboBox
    Left = 226
    Top = 7
    Width = 93
    Height = 21
    Style = csDropDownList
    DropDownCount = 16
    Enabled = False
    ItemIndex = 0
    TabOrder = 4
    Text = 'RTCM 2'
    Items.Strings = (
      'RTCM 2'
      'RTCM 3')
  end
  object Options: TEdit
    Left = 52
    Top = 83
    Width = 121
    Height = 21
    TabOrder = 6
  end
  object OutMsgs: TEdit
    Left = 6
    Top = 50
    Width = 348
    Height = 21
    TabOrder = 5
    Text = '1004(1),1012(1)'
  end
end
