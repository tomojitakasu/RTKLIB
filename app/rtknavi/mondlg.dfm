object MonitorDialog: TMonitorDialog
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  Caption = 'RTK Monitor'
  ClientHeight = 283
  ClientWidth = 464
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
    Width = 447
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
    Width = 447
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
    Width = 464
    Height = 24
    Align = alTop
    TabOrder = 1
    object Label: TLabel
      Left = 180
      Top = 5
      Width = 3
      Height = 13
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clGray
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object BtnPause: TSpeedButton
      Left = 120
      Top = 0
      Width = 24
      Height = 23
      Hint = 'Pause'
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
      Visible = False
    end
    object BtnDown: TSpeedButton
      Left = 144
      Top = 0
      Width = 24
      Height = 23
      Hint = 'Scroll Down'
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
      Visible = False
      OnClick = BtnDownClick
    end
    object BtnClear: TSpeedButton
      Left = 96
      Top = 0
      Width = 24
      Height = 23
      Hint = 'Clear'
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
      Visible = False
      OnClick = BtnClearClick
    end
    object Panel2: TPanel
      Left = 403
      Top = 1
      Width = 60
      Height = 22
      Align = alRight
      BevelOuter = bvNone
      TabOrder = 1
      object BtnClose: TButton
        Left = 0
        Top = 0
        Width = 60
        Height = 22
        Align = alClient
        Caption = '&Close'
        ModalResult = 1
        TabOrder = 0
        OnClick = BtnCloseClick
      end
    end
    object Type: TComboBox
      Left = 2
      Top = 1
      Width = 92
      Height = 21
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
        'Nav GPS'
        'Nav GLONASS'
        'Nav Galileo'
        'Nav QZSS'
        'Nav BeiDou'
        'Nav GEO'
        'Time/Iono'
        'Streams'
        'Sat GPS'
        'Sat GLONASS'
        'Sat Galileo'
        'Sat QZSS'
        'Sat BeiDou'
        'Sat GEO'
        'States'
        'Covariance'
        'SBAS Msgs'
        'SBAS Long'
        'SBAS Iono'
        'SBAS Fast'
        'RTCM Msgs'
        'RTCM DGPS'
        'RTCM SSR'
        'LEX Msgs'
        'LEX Eph/Clock'
        'LEX Iono'
        'Iono Correction'
        '(1) Rover'
        '(2) Base Station'
        '(3) Correction'
        '(4) Solution1'
        '(5) Solution2'
        'Error/Warning')
    end
    object SelEph: TComboBox
      Left = 162
      Top = 1
      Width = 66
      Height = 21
      Hint = 'Stream'
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 2
      Text = 'Current'
      Visible = False
      Items.Strings = (
        'Current'
        'Previous')
    end
    object SelStr: TComboBox
      Left = 95
      Top = 1
      Width = 80
      Height = 21
      Hint = 'Stream'
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 3
      Text = '(1) Rover'
      Visible = False
      Items.Strings = (
        '(1) Rover'
        '(2) Base Station'
        '(3) Correction'
        '(0) Combined')
    end
    object SelSat: TComboBox
      Left = 95
      Top = 1
      Width = 66
      Height = 21
      Hint = 'Stream'
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 4
      Text = 'All'
      Visible = False
      Items.Strings = (
        'All'
        'Only OK')
    end
    object SelIon: TComboBox
      Left = 95
      Top = 1
      Width = 80
      Height = 21
      Hint = 'Stream'
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 5
      Text = 'OFF'
      Visible = False
      Items.Strings = (
        'OFF'
        'Broadcast'
        'SBAS'
        'Dual-Freq'
        'Estimate STEC'
        'IONEX TEC'
        'QZS Broadcast'
        'QZS LEX')
    end
    object SelObs: TComboBox
      Left = 95
      Top = 1
      Width = 80
      Height = 21
      Hint = 'Stream'
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 6
      Text = 'Normal'
      Visible = False
      OnChange = SelObsChange
      Items.Strings = (
        'Normal'
        'Extended')
    end
    object SelFmt: TComboBox
      Left = 171
      Top = 1
      Width = 93
      Height = 21
      Hint = 'Stream'
      Style = csDropDownList
      DropDownCount = 32
      ItemIndex = 0
      TabOrder = 7
      Text = 'HEX'
      Visible = False
      OnChange = SelFmtChange
      Items.Strings = (
        'HEX'
        'ASCII')
    end
  end
  object Scroll: TScrollBar
    Left = 447
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
