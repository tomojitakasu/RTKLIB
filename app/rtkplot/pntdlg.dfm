object PntDialog: TPntDialog
  Left = 0
  Top = 0
  BorderStyle = bsDialog
  Caption = 'Waypoints'
  ClientHeight = 216
  ClientWidth = 361
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
    Top = 0
    Width = 361
    Height = 17
    Align = alTop
    BevelOuter = bvNone
    TabOrder = 6
    object Label2: TLabel
      Left = 266
      Top = 2
      Width = 54
      Height = 13
      Caption = 'Point Name'
    end
    object Label1: TLabel
      Left = 8
      Top = 2
      Width = 55
      Height = 13
      Caption = 'Latitude ('#176')'
    end
    object Label3: TLabel
      Left = 96
      Top = 2
      Width = 63
      Height = 13
      Caption = 'Longitude ('#176')'
    end
    object Label4: TLabel
      Left = 190
      Top = 2
      Width = 50
      Height = 13
      Caption = 'Height (m)'
    end
  end
  object BtnDel: TButton
    Left = 180
    Top = 192
    Width = 57
    Height = 23
    Caption = '&Delete'
    TabOrder = 5
    OnClick = BtnDelClick
  end
  object Panel2: TPanel
    Left = 0
    Top = 17
    Width = 361
    Height = 172
    Align = alTop
    BevelInner = bvLowered
    BevelOuter = bvNone
    Caption = 'Panel2'
    TabOrder = 7
    object PntList: TStringGrid
      Left = 1
      Top = 1
      Width = 359
      Height = 170
      Align = alClient
      BorderStyle = bsNone
      ColCount = 4
      DefaultRowHeight = 17
      FixedCols = 0
      RowCount = 10
      FixedRows = 0
      GridLineWidth = 0
      Options = [goFixedVertLine, goFixedHorzLine, goVertLine, goEditing]
      ScrollBars = ssNone
      TabOrder = 0
      ColWidths = (
        89
        95
        76
        99)
    end
  end
  object BtnCancel: TButton
    Left = 302
    Top = 192
    Width = 57
    Height = 23
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 0
  end
  object BtnLoad: TButton
    Left = 2
    Top = 192
    Width = 57
    Height = 23
    Caption = '&Load'
    TabOrder = 2
    OnClick = BtnLoadClick
  end
  object BtnAdd: TButton
    Left = 122
    Top = 192
    Width = 57
    Height = 23
    Caption = '&Add'
    TabOrder = 4
    OnClick = BtnAddClick
  end
  object BtnSave: TButton
    Left = 60
    Top = 192
    Width = 57
    Height = 23
    Caption = '&Save'
    TabOrder = 3
    OnClick = BtnSaveClick
  end
  object BtnOk: TButton
    Left = 244
    Top = 192
    Width = 57
    Height = 23
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 1
    OnClick = BtnOkClick
  end
  object OpenDialog: TOpenDialog
    Filter = 'All Files (*.*)|*.*|Position Files (*.pos)|*.pos'
    Options = [ofHideReadOnly, ofNoChangeDir, ofEnableSizing]
    OptionsEx = [ofExNoPlacesBar]
    Title = 'Points File'
    Left = 278
    Top = 162
  end
  object SaveDialog: TSaveDialog
    Filter = 'All Files (*.*)|*.*|Position Files (*.pos)|*.pos'
    Options = [ofHideReadOnly, ofNoChangeDir, ofEnableSizing]
    OptionsEx = [ofExNoPlacesBar]
    Title = 'Points File'
    Left = 308
    Top = 160
  end
end
