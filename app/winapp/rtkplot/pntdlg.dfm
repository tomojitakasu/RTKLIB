object PntDialog: TPntDialog
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  Caption = 'Waypoint'
  ClientHeight = 344
  ClientWidth = 374
  Color = clWhite
  Constraints.MaxWidth = 390
  Constraints.MinHeight = 200
  Constraints.MinWidth = 390
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
    Width = 374
    Height = 17
    Align = alTop
    BevelOuter = bvNone
    TabOrder = 0
    ExplicitWidth = 294
    object Label2: TLabel
      Left = 290
      Top = 2
      Width = 27
      Height = 13
      Caption = 'Name'
    end
    object Label1: TLabel
      Left = 15
      Top = 2
      Width = 55
      Height = 13
      Caption = 'Latitude ('#176')'
    end
    object Label3: TLabel
      Left = 105
      Top = 2
      Width = 63
      Height = 13
      Caption = 'Longitude ('#176')'
    end
    object Label4: TLabel
      Left = 200
      Top = 2
      Width = 50
      Height = 13
      Caption = 'Height (m)'
    end
  end
  object Panel2: TPanel
    Left = 0
    Top = 17
    Width = 374
    Height = 290
    Align = alClient
    BevelInner = bvLowered
    BevelOuter = bvNone
    Caption = 'Panel2'
    TabOrder = 1
    ExplicitWidth = 294
    object PntList: TStringGrid
      Left = 1
      Top = 1
      Width = 372
      Height = 288
      Align = alClient
      BorderStyle = bsNone
      Color = clSilver
      ColCount = 4
      DefaultRowHeight = 17
      FixedColor = clWhite
      FixedCols = 0
      RowCount = 10
      FixedRows = 0
      GradientEndColor = clBlack
      Options = [goFixedVertLine, goFixedHorzLine, goVertLine, goEditing]
      ScrollBars = ssVertical
      TabOrder = 0
      OnClick = PntListClick
      OnDblClick = PntListDblClick
      OnSetEditText = PntListSetEditText
      ExplicitTop = 0
      ExplicitWidth = 382
      ColWidths = (
        89
        95
        76
        64)
    end
  end
  object Panel3: TPanel
    Left = 0
    Top = 307
    Width = 374
    Height = 37
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 2
    ExplicitWidth = 294
    object BtnAdd: TButton
      Left = 3
      Top = 6
      Width = 75
      Height = 29
      Caption = '&Add'
      TabOrder = 0
      OnClick = BtnAddClick
    end
    object BtnClose: TButton
      Left = 296
      Top = 6
      Width = 75
      Height = 29
      Caption = '&Close'
      TabOrder = 1
      OnClick = BtnCloseClick
    end
    object BtnDel: TButton
      Left = 80
      Top = 6
      Width = 75
      Height = 29
      Caption = '&Delete'
      TabOrder = 2
      OnClick = BtnDelClick
    end
  end
  object OpenDialog: TOpenDialog
    Filter = 'All Files (*.*)|*.*|Position Files (*.pos)|*.pos'
    Options = [ofHideReadOnly, ofNoChangeDir, ofEnableSizing]
    OptionsEx = [ofExNoPlacesBar]
    Title = 'Points File'
    Left = 105
    Top = 169
  end
  object SaveDialog: TSaveDialog
    Filter = 'All Files (*.*)|*.*|Position Files (*.pos)|*.pos'
    Options = [ofHideReadOnly, ofNoChangeDir, ofEnableSizing]
    OptionsEx = [ofExNoPlacesBar]
    Title = 'Points File'
    Left = 136
    Top = 168
  end
end
