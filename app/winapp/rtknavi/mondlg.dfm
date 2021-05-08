object MonitorDialog: TMonitorDialog
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  Caption = 'RTK Monitor'
  ClientHeight = 283
  ClientWidth = 893
  Color = clWhite
  Constraints.MinHeight = 160
  Constraints.MinWidth = 200
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  OnClose = FormClose
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object Console: TPaintBox
    Left = 0
    Top = 24
    Width = 876
    Height = 259
    Align = alClient
    Color = clWhite
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Courier New'
    Font.Style = []
    ParentColor = False
    ParentFont = False
    OnPaint = ConsolePaint
    ExplicitLeft = 128
    ExplicitTop = 172
    ExplicitWidth = 105
    ExplicitHeight = 105
  end
  object Tbl: TStringGrid
    Left = 0
    Top = 24
    Width = 876
    Height = 259
    TabStop = False
    Align = alClient
    BevelInner = bvNone
    BorderStyle = bsNone
    Color = clWhite
    DefaultColWidth = 100
    DefaultRowHeight = 15
    FixedCols = 0
    RowCount = 10
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    Options = [goFixedVertLine, goFixedHorzLine, goVertLine, goRangeSelect, goColSizing]
    ParentFont = False
    TabOrder = 0
    ColWidths = (
      100
      100
      100
      100
      100)
    RowHeights = (
      15
      15
      15
      15
      15
      15
      15
      15
      15
      15)
  end
  object Panel1: TPanel
    Left = 0
    Top = 0
    Width = 893
    Height = 24
    Align = alTop
    TabOrder = 1
    object Label: TLabel
      Left = 374
      Top = 5
      Width = 3
      Height = 13
      Color = clWhite
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clBlack
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object Type: TComboBox
      Left = 1
      Top = 2
      Width = 90
      Height = 21
      Margins.Left = 1
      Margins.Top = 1
      Margins.Right = 1
      Margins.Bottom = 0
      AutoComplete = False
      Style = csDropDownList
      DropDownCount = 40
      ItemIndex = 0
      TabOrder = 0
      TabStop = False
      Text = 'RTK'
      OnChange = TypeChange
      Items.Strings = (
        'RTK'
        'Obs Data'
        'Nav Data'
        'Time/Iono'
        'Streams'
        'Sat Status'
        'States'
        'Covariance'
        'SBAS Msgs'
        'SBAS Long'
        'SBAS Iono'
        'SBAS Fast'
        'RTCM Msgs'
        'RTCM DGPS'
        'RTCM SSR'
        'Station Info'
        'Input'
        'Output'
        'Error/Warning')
    end
    object SelEph: TComboBox
      AlignWithMargins = True
      Left = 276
      Top = 2
      Width = 90
      Height = 21
      Hint = 'Stream'
      Margins.Left = 1
      Margins.Top = 1
      Margins.Right = 1
      Margins.Bottom = 0
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 1
      Text = 'Current 1'
      Visible = False
      Items.Strings = (
        'Current 1'
        'Current 2'
        'Previous 1'
        'Previous 2')
    end
    object SelStr: TComboBox
      AlignWithMargins = True
      Left = 92
      Top = 2
      Width = 90
      Height = 21
      Hint = 'Stream'
      Margins.Left = 1
      Margins.Top = 1
      Margins.Right = 1
      Margins.Bottom = 0
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 2
      Text = '(1) Rover'
      Visible = False
      OnChange = SelStrChange
      Items.Strings = (
        '(1) Rover'
        '(2) Base Station'
        '(3) Correction')
    end
    object SelSat: TComboBox
      AlignWithMargins = True
      Left = 184
      Top = 2
      Width = 90
      Height = 21
      Hint = 'Stream'
      Margins.Left = 1
      Margins.Top = 1
      Margins.Right = 1
      Margins.Bottom = 0
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 3
      Text = 'ALL'
      Visible = False
      Items.Strings = (
        'ALL'
        'Only OK')
    end
    object SelObs: TComboBox
      AlignWithMargins = True
      Left = 184
      Top = 2
      Width = 90
      Height = 21
      Hint = 'Stream'
      Margins.Left = 1
      Margins.Top = 1
      Margins.Right = 1
      Margins.Bottom = 0
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 4
      Text = 'Normal'
      Visible = False
      OnChange = SelObsChange
      Items.Strings = (
        'Normal'
        'Extended')
    end
    object SelFmt: TComboBox
      AlignWithMargins = True
      Left = 184
      Top = 2
      Width = 90
      Height = 21
      Hint = 'Stream'
      Margins.Left = 1
      Margins.Top = 1
      Margins.Right = 1
      Margins.Bottom = 0
      Style = csDropDownList
      DropDownCount = 32
      ItemIndex = 1
      TabOrder = 5
      Text = 'ASCII'
      Visible = False
      OnChange = SelFmtChange
      Items.Strings = (
        'HEX'
        'ASCII')
    end
    object SelSys: TComboBox
      AlignWithMargins = True
      Left = 92
      Top = 2
      Width = 90
      Height = 21
      Hint = 'Stream'
      Margins.Left = 1
      Margins.Top = 1
      Margins.Right = 1
      Margins.Bottom = 0
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 6
      Text = 'ALL'
      Visible = False
      Items.Strings = (
        'ALL'
        'GPS'
        'GLONASS'
        'Galileo'
        'QZSS'
        'BDS'
        'NavIC'
        'SBAS')
    end
    object SelSys2: TComboBox
      AlignWithMargins = True
      Left = 92
      Top = 2
      Width = 90
      Height = 21
      Hint = 'Stream'
      Margins.Left = 1
      Margins.Top = 1
      Margins.Right = 1
      Margins.Bottom = 0
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 7
      Text = 'GPS'
      Visible = False
      Items.Strings = (
        'GPS'
        'GLONASS'
        'Galileo'
        'QZSS'
        'BDS'
        'NavIC'
        'SBAS')
    end
    object SelStr2: TComboBox
      AlignWithMargins = True
      Left = 92
      Top = 2
      Width = 90
      Height = 21
      Hint = 'Stream'
      Margins.Left = 1
      Margins.Top = 1
      Margins.Right = 1
      Margins.Bottom = 0
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 8
      Text = '(4) Solution 1'
      OnChange = SelStr2Change
      Items.Strings = (
        '(4) Solution 1'
        '(5) Solution 2')
    end
    object BtnClose: TButton
      AlignWithMargins = True
      Left = 821
      Top = 1
      Width = 70
      Height = 22
      Margins.Left = 2
      Margins.Top = 0
      Margins.Right = 1
      Margins.Bottom = 0
      Align = alRight
      Caption = '&Close'
      ModalResult = 1
      TabOrder = 9
      OnClick = BtnCloseClick
    end
    object Panel2: TPanel
      Left = 276
      Top = 1
      Width = 72
      Height = 22
      AutoSize = True
      BevelOuter = bvNone
      TabOrder = 10
      object BtnClear: TSpeedButton
        Left = 0
        Top = 0
        Width = 24
        Height = 22
        Hint = 'Clear'
        Align = alLeft
        Flat = True
        Glyph.Data = {
          3E020000424D3E0200000000000036000000280000000D0000000D0000000100
          1800000000000802000000000000000000000000000000000000FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFF000000FFFFFFFFFFFF00FFFFFFFFFFFFB4B4B4000000FFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFF000000B4B4B4FFFFFFFFFFFF00FFFFFFFFFFFF
          FFFFFFB4B4B4000000FFFFFFFFFFFFFFFFFF000000B4B4B4FFFFFFFFFFFFFFFF
          FF00FFFFFFFFFFFFFFFFFFFFFFFFB4B4B4000000FFFFFF000000B4B4B4FFFFFF
          FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFB4B4B4000000B4
          B4B4FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF000000B4B4B4000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
          FFFFFFFFFFFF000000B4B4B4FFFFFFB4B4B4000000FFFFFFFFFFFFFFFFFFFFFF
          FF00FFFFFFFFFFFFFFFFFF000000B4B4B4FFFFFFFFFFFFFFFFFFB4B4B4000000
          FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF000000B4B4B4FFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFB4B4B4000000FFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF00}
        ParentShowHint = False
        ShowHint = True
        OnClick = BtnClearClick
        ExplicitTop = -1
      end
      object BtnDown: TSpeedButton
        Left = 48
        Top = 0
        Width = 24
        Height = 22
        Hint = 'Scroll Down'
        Align = alLeft
        AllowAllUp = True
        GroupIndex = 3
        Down = True
        Flat = True
        Glyph.Data = {
          3E020000424D3E0200000000000036000000280000000D0000000D0000000100
          1800000000000802000000000000000000000000000000000000FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF00FFFFFF808080808080808080808080808080808080808080808080808080
          808080808080FFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF000000000000000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
          FFFFFFFFFFFF000000000000000000000000000000FFFFFFFFFFFFFFFFFFFFFF
          FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF00}
        ParentShowHint = False
        ShowHint = True
        OnClick = BtnDownClick
        ExplicitLeft = 109
        ExplicitTop = -2
      end
      object BtnPause: TSpeedButton
        Left = 24
        Top = 0
        Width = 24
        Height = 22
        Hint = 'Pause'
        Align = alLeft
        AllowAllUp = True
        GroupIndex = 2
        Flat = True
        Glyph.Data = {
          3E020000424D3E0200000000000036000000280000000D0000000D0000000100
          1800000000000802000000000000000000000000000000000000FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
          FFFFFFFFFFFF000000000000FFFFFF000000000000FFFFFFFFFFFFFFFFFFFFFF
          FF00FFFFFFFFFFFFFFFFFFFFFFFF000000000000FFFFFF000000000000FFFFFF
          FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFFFF000000000000FFFFFF00
          0000000000FFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFFFF0000
          00000000FFFFFF000000000000FFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
          FFFFFFFFFFFF000000000000FFFFFF000000000000FFFFFFFFFFFFFFFFFFFFFF
          FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF00}
        ParentShowHint = False
        ShowHint = True
        ExplicitLeft = 21
        ExplicitTop = 1
      end
    end
  end
  object Scroll: TScrollBar
    Left = 876
    Top = 24
    Width = 17
    Height = 259
    Align = alRight
    Ctl3D = True
    DoubleBuffered = False
    Kind = sbVertical
    PageSize = 0
    ParentCtl3D = False
    ParentDoubleBuffered = False
    ParentShowHint = False
    ShowHint = False
    TabOrder = 2
    TabStop = False
    Visible = False
    OnChange = ScrollChange
  end
  object Timer1: TTimer
    Interval = 100
    OnTimer = Timer1Timer
    Left = 8
    Top = 286
  end
  object Timer2: TTimer
    Interval = 100
    OnTimer = Timer2Timer
    Left = 36
    Top = 286
  end
end
