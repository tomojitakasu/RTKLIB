object FileOptDialog: TFileOptDialog
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'File Options'
  ClientHeight = 77
  ClientWidth = 399
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
    Left = 109
    Top = 54
    Width = 8
    Height = 13
    Caption = '+'
    Visible = False
  end
  object Label3: TLabel
    Left = 175
    Top = 54
    Width = 5
    Height = 13
    Caption = 's'
    Visible = False
  end
  object Label5: TLabel
    Left = 178
    Top = 54
    Width = 7
    Height = 13
    Caption = 'H'
  end
  object Label4: TLabel
    Left = 78
    Top = 54
    Width = 49
    Height = 13
    Caption = 'Swap Intv'
  end
  object BtnKey: TSpeedButton
    Left = 194
    Top = 50
    Width = 17
    Height = 22
    Caption = '?'
    Flat = True
    OnClick = BtnKeyClick
  end
  object TimeStart: TEdit
    Left = 122
    Top = 50
    Width = 51
    Height = 21
    TabOrder = 5
    Text = '0'
    Visible = False
  end
  object Panel1: TPanel
    Left = 0
    Top = 0
    Width = 399
    Height = 43
    Align = alTop
    BevelOuter = bvNone
    TabOrder = 2
    ExplicitWidth = 384
    object Label1: TLabel
      Left = 8
      Top = 4
      Width = 41
      Height = 13
      Caption = 'File Path'
    end
    object BtnFilePath: TButton
      Left = 372
      Top = 17
      Width = 25
      Height = 23
      Caption = '...'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -9
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 0
      OnClick = BtnFilePathClick
    end
    object FilePath: TEdit
      Left = 6
      Top = 18
      Width = 365
      Height = 21
      TabOrder = 1
    end
  end
  object BtnCancel: TButton
    Left = 316
    Top = 45
    Width = 81
    Height = 29
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 0
  end
  object BtnOk: TButton
    Left = 232
    Top = 45
    Width = 81
    Height = 29
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 1
    OnClick = BtnOkClick
  end
  object ChkTimeTag: TCheckBox
    Left = 6
    Top = 52
    Width = 63
    Height = 17
    Caption = 'Time'
    TabOrder = 3
    OnClick = ChkTimeTagClick
  end
  object TimeSpeed: TComboBox
    Left = 54
    Top = 50
    Width = 51
    Height = 21
    ItemIndex = 3
    TabOrder = 4
    Text = 'x1'
    Visible = False
    Items.Strings = (
      'x0.1'
      'x0.2'
      'x0.5'
      'x1'
      'x2'
      'x5'
      'x10')
  end
  object SwapIntv: TComboBox
    Left = 130
    Top = 50
    Width = 45
    Height = 21
    DropDownCount = 10
    TabOrder = 6
    Items.Strings = (
      ''
      '0.25'
      '0.5'
      '1'
      '2'
      '3'
      '6'
      '12'
      '24')
  end
  object Chk64Bit: TCheckBox
    Left = 186
    Top = 52
    Width = 41
    Height = 17
    Caption = '64bit'
    TabOrder = 7
  end
  object SaveDialog: TSaveDialog
    Filter = 'All File (*.*)|*.*|Position File (*.pos)|*.pos'
    Options = [ofHideReadOnly, ofNoChangeDir, ofEnableSizing]
    Title = 'Output File'
    Left = 256
    Top = 8
  end
  object OpenDialog: TOpenDialog
    Filter = 'All File (*.*)|*.*|Position File (*.pos)|*.pos'
    Options = [ofHideReadOnly, ofNoChangeDir, ofEnableSizing]
    Title = 'Input File'
    Left = 284
    Top = 8
  end
end
