object CmdOptDialog: TCmdOptDialog
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'Serial/TCP Commands'
  ClientHeight = 264
  ClientWidth = 318
  Color = clBtnFace
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
  object BtnOk: TButton
    Left = 150
    Top = 240
    Width = 81
    Height = 23
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 1
    OnClick = BtnOkClick
  end
  object BtnCancel: TButton
    Left = 234
    Top = 240
    Width = 81
    Height = 23
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 0
  end
  object ChkOpenCmd: TCheckBox
    Left = 2
    Top = 2
    Width = 151
    Height = 17
    Caption = 'Commands at startup'
    TabOrder = 2
    OnClick = ChkOpenCmdClick
  end
  object ChkCloseCmd: TCheckBox
    Left = 2
    Top = 120
    Width = 157
    Height = 17
    Caption = 'Commands at shutdown'
    TabOrder = 4
    OnClick = ChkCloseCmdClick
  end
  object CloseCmd: TMemo
    Left = 2
    Top = 136
    Width = 311
    Height = 101
    ScrollBars = ssVertical
    TabOrder = 5
    WordWrap = False
  end
  object OpenCmd: TMemo
    Left = 2
    Top = 18
    Width = 311
    Height = 101
    ScrollBars = ssVertical
    TabOrder = 3
    WordWrap = False
  end
  object BtnLoad: TButton
    Left = 4
    Top = 240
    Width = 65
    Height = 23
    Caption = '&Load...'
    TabOrder = 6
    OnClick = BtnLoadClick
  end
  object BtnSave: TButton
    Left = 72
    Top = 240
    Width = 65
    Height = 23
    Caption = '&Save...'
    TabOrder = 7
    OnClick = BtnSaveClick
  end
  object SaveDialog: TSaveDialog
    Filter = 'Command File (*.cmd)|*.cmd|All File (*.*)|*.*'
    Options = [ofHideReadOnly, ofNoChangeDir, ofEnableSizing]
    Title = 'Save Serial Commands'
    Left = 264
    Top = 204
  end
  object OpenDialog: TOpenDialog
    Filter = 'Command File (*.cmd)|*.cmd|All File (*.*)|*.*'
    Options = [ofHideReadOnly, ofNoChangeDir, ofEnableSizing]
    Title = 'Load Serial Commands'
    Left = 236
    Top = 204
  end
end
