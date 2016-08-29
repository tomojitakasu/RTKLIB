object RefDialog: TRefDialog
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  Caption = 'Stations'
  ClientHeight = 443
  ClientWidth = 433
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
  object StaList: TStringGrid
    Left = 0
    Top = 0
    Width = 433
    Height = 412
    Margins.Top = 1
    Margins.Bottom = 1
    Align = alClient
    ColCount = 7
    DefaultRowHeight = 16
    FixedCols = 0
    RowCount = 2
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    GridLineWidth = 0
    Options = [goFixedVertLine, goFixedHorzLine, goVertLine, goHorzLine, goColSizing, goRowSelect]
    ParentFont = False
    ScrollBars = ssVertical
    TabOrder = 0
    OnDblClick = StaListDblClick
    OnMouseDown = StaListMouseDown
    ColWidths = (
      31
      78
      83
      64
      39
      70
      69)
  end
  object Panel1: TPanel
    Left = 0
    Top = 412
    Width = 433
    Height = 31
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 1
    object BtnLoad: TButton
      Left = 2
      Top = 2
      Width = 70
      Height = 27
      Caption = '&Load'
      TabOrder = 0
      OnClick = BtnLoadClick
    end
    object Panel2: TPanel
      Left = 280
      Top = 0
      Width = 153
      Height = 31
      Align = alRight
      BevelOuter = bvNone
      TabOrder = 1
      object BtnOK: TButton
        Left = 4
        Top = 2
        Width = 70
        Height = 27
        Caption = '&OK'
        ModalResult = 1
        TabOrder = 0
        OnClick = BtnOKClick
      end
      object BtnCancel: TButton
        Left = 80
        Top = 2
        Width = 70
        Height = 27
        Caption = '&Cancel'
        ModalResult = 2
        TabOrder = 1
      end
    end
    object BtnFind: TButton
      Left = 202
      Top = 2
      Width = 45
      Height = 27
      Caption = 'Find'
      TabOrder = 2
      OnClick = BtnFindClick
    end
    object FindStr: TEdit
      Left = 112
      Top = 4
      Width = 89
      Height = 21
      TabOrder = 3
      OnKeyPress = FindStrKeyPress
    end
  end
  object OpenDialog: TOpenDialog
    Filter = 'Position File (*.pos,*.snx)|*.pos;*.snx|All (*.*)|*.*'
    Options = [ofHideReadOnly, ofNoChangeDir, ofEnableSizing]
    Left = 222
    Top = 336
  end
end
