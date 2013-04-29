object StaListDialog: TStaListDialog
  Left = 0
  Top = 0
  BorderStyle = bsDialog
  Caption = 'Stations'
  ClientHeight = 302
  ClientWidth = 214
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
  object Panel1: TPanel
    Left = 0
    Top = 279
    Width = 214
    Height = 23
    Align = alBottom
    BevelInner = bvRaised
    BevelOuter = bvNone
    TabOrder = 0
    ExplicitTop = 291
    ExplicitWidth = 213
    object BtnLoad: TButton
      Left = 1
      Top = 0
      Width = 53
      Height = 23
      Caption = 'Load...'
      TabOrder = 1
      OnClick = BtnLoadClick
    end
    object BtnSave: TButton
      Left = 54
      Top = 0
      Width = 53
      Height = 23
      Caption = '&Save...'
      TabOrder = 2
      OnClick = BtnSaveClick
    end
    object BtnOk: TButton
      Left = 107
      Top = 0
      Width = 52
      Height = 23
      Caption = '&OK'
      ModalResult = 1
      TabOrder = 3
      OnClick = BtnOkClick
    end
    object BtnCancel: TButton
      Left = 159
      Top = 0
      Width = 53
      Height = 23
      Caption = '&Cancel'
      ModalResult = 2
      TabOrder = 0
    end
  end
  object StaList: TMemo
    Left = 0
    Top = 0
    Width = 214
    Height = 279
    Align = alClient
    BevelInner = bvNone
    BevelOuter = bvNone
    ScrollBars = ssVertical
    TabOrder = 1
    ExplicitWidth = 213
    ExplicitHeight = 291
  end
  object OpenDialog: TOpenDialog
    Filter = 'Text File (*.txt)|*.txt|All (*.*)|*.*'
    Title = 'Statiion List File'
    Left = 89
    Top = 110
  end
  object SaveDialog: TSaveDialog
    Filter = 'Text File (*.txt)|*.txt|All (*.*)|*.*'
    Options = [ofOverwritePrompt, ofHideReadOnly, ofEnableSizing]
    Title = 'Statiion List File'
    Left = 120
    Top = 111
  end
end
