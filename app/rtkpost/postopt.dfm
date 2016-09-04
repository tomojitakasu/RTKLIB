object OptDialog: TOptDialog
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'Options'
  ClientHeight = 295
  ClientWidth = 411
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
  object Label5: TLabel
    Left = 12
    Top = 54
    Width = 69
    Height = 13
    Caption = 'Correction File'
  end
  object Label82: TLabel
    Left = 92
    Top = 3
    Width = 6
    Height = 13
    Caption = 'a'
  end
  object Label83: TLabel
    Left = 138
    Top = 3
    Width = 6
    Height = 13
    Caption = 'b'
  end
  object Label86: TLabel
    Left = 244
    Top = 3
    Width = 6
    Height = 13
    Caption = 'b'
  end
  object Label87: TLabel
    Left = 198
    Top = 3
    Width = 6
    Height = 13
    Caption = 'a'
  end
  object BtnCancel: TButton
    Left = 310
    Top = 264
    Width = 99
    Height = 29
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 1
  end
  object BtnOk: TButton
    Left = 211
    Top = 264
    Width = 99
    Height = 29
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 0
    OnClick = BtnOkClick
  end
  object BtnSave: TButton
    Left = 100
    Top = 264
    Width = 99
    Height = 29
    Caption = '&Save...'
    TabOrder = 3
    OnClick = BtnSaveClick
  end
  object BtnLoad: TButton
    Left = 1
    Top = 264
    Width = 99
    Height = 29
    Caption = '&Load...'
    TabOrder = 2
    OnClick = BtnLoadClick
  end
  object Misc: TPageControl
    Left = 0
    Top = 0
    Width = 411
    Height = 263
    ActivePage = TabSheet1
    TabOrder = 4
    object TabSheet1: TTabSheet
      Caption = 'Setting&1'
      ExplicitHeight = 234
      object Label3: TLabel
        Left = 24
        Top = 73
        Width = 183
        Height = 13
        Caption = 'Rec Dynamics / Earth Tides Correction'
      end
      object Label8: TLabel
        Left = 24
        Top = 95
        Width = 108
        Height = 13
        Caption = 'Ionosphere Correction'
      end
      object LabelPosMode: TLabel
        Left = 24
        Top = 7
        Width = 80
        Height = 13
        Caption = 'Positioning Mode'
      end
      object LabelFreq: TLabel
        Left = 24
        Top = 29
        Width = 119
        Height = 13
        Caption = 'Frequencies / Filter Type'
      end
      object LabelElMask: TLabel
        Left = 23
        Top = 51
        Width = 179
        Height = 13
        Caption = 'Elevation Mask ('#176') / SNR Mask (dBHz)'
      end
      object Label32: TLabel
        Left = 24
        Top = 139
        Width = 119
        Height = 13
        Caption = 'Satellite Ephemeris/Clock'
      end
      object Label35: TLabel
        Left = 24
        Top = 188
        Width = 176
        Height = 13
        Caption = 'Excluded Satellites (+PRN: Included)'
      end
      object Label9: TLabel
        Left = 23
        Top = 117
        Width = 114
        Height = 13
        Caption = 'Troposphere Correction'
      end
      object DynamicModel: TComboBox
        Left = 248
        Top = 70
        Width = 75
        Height = 21
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 5
        Text = 'OFF'
        OnChange = DynamicModelChange
        Items.Strings = (
          'OFF'
          'ON')
      end
      object IonoOpt: TComboBox
        Left = 248
        Top = 92
        Width = 152
        Height = 21
        Style = csDropDownList
        DropDownCount = 16
        ItemIndex = 0
        TabOrder = 7
        Text = 'OFF'
        OnChange = IonoOptChange
        Items.Strings = (
          'OFF'
          'Broadcast'
          'SBAS'
          'Iono-Free LC'
          'Estimate TEC'
          'IONEX TEC'
          'QZSS Broardcast'
          'QZSS LEX'
          'Input TEC')
      end
      object TropOpt: TComboBox
        Left = 248
        Top = 114
        Width = 152
        Height = 21
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 8
        Text = 'OFF'
        OnChange = TropOptChange
        Items.Strings = (
          'OFF'
          'Saastamoinen'
          'SBAS'
          'Estimate ZTD'
          'Estimate ZTD+Grad'
          'Input ZTD')
      end
      object PosMode: TComboBox
        Left = 248
        Top = 4
        Width = 152
        Height = 21
        Style = csDropDownList
        DropDownCount = 10
        ItemIndex = 0
        TabOrder = 0
        Text = 'Single'
        OnChange = PosModeChange
        Items.Strings = (
          'Single'
          'DGPS/DGNSS'
          'Kinematic'
          'Static'
          'Moving-Base'
          'Fixed'
          'PPP Kinematic'
          'PPP Static'
          'PPP Fixed')
      end
      object Freq: TComboBox
        Left = 248
        Top = 26
        Width = 75
        Height = 21
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 1
        Text = 'L1'
        OnChange = FreqChange
        Items.Strings = (
          'L1'
          'L1+L2'
          'L1+L2+L5'
          'L1+L5')
      end
      object Solution: TComboBox
        Left = 325
        Top = 26
        Width = 75
        Height = 21
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 2
        Text = 'Forward'
        Items.Strings = (
          'Forward'
          'Backward'
          'Combined')
      end
      object SatEphem: TComboBox
        Left = 248
        Top = 136
        Width = 152
        Height = 21
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 9
        Text = 'Broadcast'
        OnChange = SatEphemChange
        OnClick = SatEphemClick
        Items.Strings = (
          'Broadcast'
          'Precise'
          'Broadcast+SBAS'
          'Broadcast+SSR APC'
          'Broadcast+SSR CoM'
          'QZSS LEX')
      end
      object ExSats: TEdit
        Left = 221
        Top = 184
        Width = 179
        Height = 21
        TabOrder = 16
      end
      object NavSys1: TCheckBox
        Left = 24
        Top = 210
        Width = 49
        Height = 17
        Caption = 'GPS'
        Checked = True
        State = cbChecked
        TabOrder = 17
      end
      object NavSys2: TCheckBox
        Left = 68
        Top = 210
        Width = 71
        Height = 17
        Caption = 'GLO'
        TabOrder = 18
        OnClick = NavSys2Click
      end
      object NavSys3: TCheckBox
        Left = 114
        Top = 210
        Width = 61
        Height = 17
        Caption = 'Galileo'
        TabOrder = 19
      end
      object NavSys4: TCheckBox
        Left = 169
        Top = 210
        Width = 61
        Height = 17
        Caption = 'QZSS'
        TabOrder = 20
      end
      object NavSys5: TCheckBox
        Left = 222
        Top = 210
        Width = 61
        Height = 17
        Caption = 'SBAS'
        TabOrder = 21
      end
      object TideCorr: TComboBox
        Left = 325
        Top = 70
        Width = 75
        Height = 21
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 6
        Text = 'OFF'
        OnChange = DynamicModelChange
        Items.Strings = (
          'OFF'
          'Solid'
          'Solid/OTL')
      end
      object NavSys6: TCheckBox
        Left = 274
        Top = 210
        Width = 61
        Height = 17
        Caption = 'BeiDou'
        TabOrder = 22
        OnClick = NavSys6Click
      end
      object ElMask: TComboBox
        Left = 248
        Top = 48
        Width = 75
        Height = 21
        AutoComplete = False
        DropDownCount = 16
        TabOrder = 3
        Text = '10'
        OnChange = DynamicModelChange
        Items.Strings = (
          '0'
          '5'
          '10'
          '15'
          '20'
          '25'
          '30'
          '35'
          '40'
          '45'
          '50'
          '55'
          '60'
          '65'
          '70')
      end
      object PosOpt1: TCheckBox
        Left = 24
        Top = 162
        Width = 62
        Height = 17
        Caption = 'Sat PCV'
        TabOrder = 10
      end
      object PosOpt3: TCheckBox
        Left = 144
        Top = 162
        Width = 85
        Height = 17
        Caption = 'PhWU'
        TabOrder = 12
      end
      object PosOpt2: TCheckBox
        Left = 83
        Top = 162
        Width = 57
        Height = 17
        Caption = 'Rec PCV'
        TabOrder = 11
      end
      object PosOpt4: TCheckBox
        Left = 194
        Top = 162
        Width = 85
        Height = 17
        Caption = 'Rej Ecl'
        TabOrder = 13
      end
      object BtnMask: TButton
        Left = 324
        Top = 47
        Width = 77
        Height = 23
        Caption = '...'
        TabOrder = 4
        OnClick = BtnMaskClick
      end
      object PosOpt5: TCheckBox
        Left = 246
        Top = 162
        Width = 72
        Height = 17
        Caption = 'RAIM FDE'
        TabOrder = 14
      end
      object PosOpt6: TCheckBox
        Left = 314
        Top = 162
        Width = 72
        Height = 17
        Caption = 'DBCorr'
        TabOrder = 15
      end
      object NavSys7: TCheckBox
        Left = 328
        Top = 210
        Width = 61
        Height = 17
        Caption = 'IRNSS'
        TabOrder = 23
        OnClick = NavSys6Click
      end
    end
    object TabSheet2: TTabSheet
      Caption = 'Setting&2'
      ImageIndex = 1
      ExplicitHeight = 234
      object Label25: TLabel
        Left = 24
        Top = 7
        Width = 184
        Height = 13
        Caption = 'Integer Ambiguity Res (GPS/GLO/BDS)'
      end
      object LabelRatio: TLabel
        Left = 24
        Top = 32
        Width = 124
        Height = 13
        Caption = 'Min Ratio to Fix Ambiguity'
      end
      object Label13: TLabel
        Left = 25
        Top = 76
        Width = 164
        Height = 13
        Caption = 'Min Lock / Elevation ('#176') to Fix Amb'
      end
      object LabelHold: TLabel
        Left = 24
        Top = 98
        Width = 164
        Height = 13
        Caption = 'Min Fix / Elevation ('#176') to Hold Amb'
      end
      object Label22: TLabel
        Left = 24
        Top = 125
        Width = 173
        Height = 13
        Caption = 'Outage to Reset Amb/Slip Thres (m)'
      end
      object Label14: TLabel
        Left = 24
        Top = 147
        Width = 165
        Height = 13
        Caption = 'Max Age of Diff (s) / Sync Solution'
      end
      object Label11: TLabel
        Left = 24
        Top = 169
        Width = 176
        Height = 13
        Caption = 'Reject Threshold of GDOP/Innov (m)'
      end
      object Label37: TLabel
        Left = 24
        Top = 191
        Width = 155
        Height = 13
        Caption = 'Max # of AR Iter/# of Filter Iter'
      end
      object LabelConf: TLabel
        Left = 24
        Top = 54
        Width = 179
        Height = 13
        Caption = 'Min Confidence / Max FCB to Fix Amb'
      end
      object AmbRes: TComboBox
        Left = 248
        Top = 5
        Width = 62
        Height = 21
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 0
        Text = 'OFF'
        OnChange = AmbResChange
        Items.Strings = (
          'OFF'
          'Continuous'
          'Instantaneous'
          'Fix and Hold'
          'PPP-AR')
      end
      object ValidThresAR: TEdit
        Left = 248
        Top = 30
        Width = 152
        Height = 21
        TabOrder = 3
        Text = '3.0'
      end
      object LockCntFixAmb: TEdit
        Left = 248
        Top = 74
        Width = 75
        Height = 21
        TabOrder = 5
        Text = '5'
      end
      object OutCntResetAmb: TEdit
        Left = 248
        Top = 123
        Width = 75
        Height = 21
        TabOrder = 7
        Text = '5'
      end
      object ElMaskAR: TEdit
        Left = 325
        Top = 74
        Width = 75
        Height = 21
        TabOrder = 4
        Text = '0'
      end
      object SlipThres: TEdit
        Left = 325
        Top = 123
        Width = 75
        Height = 21
        TabOrder = 8
        Text = '0.05'
      end
      object MaxAgeDiff: TEdit
        Left = 248
        Top = 145
        Width = 75
        Height = 21
        TabOrder = 9
        Text = '30'
      end
      object RejectThres: TEdit
        Left = 325
        Top = 167
        Width = 75
        Height = 21
        TabOrder = 11
        Text = '30'
      end
      object NumIter: TEdit
        Left = 325
        Top = 189
        Width = 75
        Height = 21
        TabOrder = 13
        Text = '1'
      end
      object BaselineLen: TEdit
        Left = 248
        Top = 211
        Width = 75
        Height = 21
        TabOrder = 15
        Text = '0.0'
      end
      object BaselineSig: TEdit
        Left = 325
        Top = 211
        Width = 75
        Height = 21
        TabOrder = 16
        Text = '0.001'
      end
      object BaselineConst: TCheckBox
        Left = 24
        Top = 212
        Width = 179
        Height = 17
        Caption = 'Baseline Length Constraint (m)'
        TabOrder = 14
        OnClick = BaselineConstClick
      end
      object GloAmbRes: TComboBox
        Left = 312
        Top = 5
        Width = 43
        Height = 21
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 1
        Text = 'OFF'
        OnChange = AmbResChange
        Items.Strings = (
          'OFF'
          'ON'
          'Auto Calibration')
      end
      object FixCntHoldAmb: TEdit
        Left = 248
        Top = 96
        Width = 75
        Height = 21
        TabOrder = 17
        Text = '10'
      end
      object ElMaskHold: TEdit
        Left = 325
        Top = 96
        Width = 75
        Height = 21
        TabOrder = 6
        Text = '0'
      end
      object RejectGdop: TEdit
        Left = 248
        Top = 167
        Width = 75
        Height = 21
        TabOrder = 10
        Text = '30'
      end
      object ThresAR2: TEdit
        Left = 248
        Top = 52
        Width = 75
        Height = 21
        TabOrder = 18
        Text = '0.99995'
      end
      object ThresAR3: TEdit
        Left = 325
        Top = 52
        Width = 75
        Height = 21
        TabOrder = 19
        Text = '0.20'
      end
      object SyncSol: TComboBox
        Left = 325
        Top = 145
        Width = 75
        Height = 21
        Style = csDropDownList
        Enabled = False
        ItemIndex = 1
        TabOrder = 20
        Text = 'ON'
        OnChange = AmbResChange
        Items.Strings = (
          'OFF'
          'ON'
          'Auto Calibration')
      end
      object BdsAmbRes: TComboBox
        Left = 357
        Top = 5
        Width = 43
        Height = 21
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 2
        Text = 'OFF'
        OnChange = AmbResChange
        Items.Strings = (
          'OFF'
          'ON')
      end
      object ARIter: TEdit
        Left = 248
        Top = 189
        Width = 75
        Height = 21
        TabOrder = 12
        Text = '1'
      end
    end
    object TabSheet3: TTabSheet
      Caption = 'O&utput'
      ImageIndex = 2
      ExplicitHeight = 234
      object LabelSolFormat: TLabel
        Left = 24
        Top = 9
        Width = 75
        Height = 13
        Caption = 'Solution Format'
      end
      object LabelTimeFormat: TLabel
        Left = 24
        Top = 53
        Width = 134
        Height = 13
        Caption = 'Time Format / # of Decimals'
      end
      object LabelLatLonFormat: TLabel
        Left = 24
        Top = 75
        Width = 209
        Height = 13
        Caption = 'Latitude Longitude Format / Field Separator'
      end
      object LabelFieldSep: TLabel
        Left = 24
        Top = 97
        Width = 215
        Height = 13
        Caption = 'Output Single if Sol Outage / Max Sol Std (m)'
      end
      object Label2: TLabel
        Left = 24
        Top = 119
        Width = 72
        Height = 13
        Caption = 'Datum / Height'
      end
      object Label18: TLabel
        Left = 24
        Top = 141
        Width = 58
        Height = 13
        Caption = 'Geoid Model'
      end
      object Label20: TLabel
        Left = 24
        Top = 31
        Width = 210
        Height = 13
        Caption = 'Output Header / Output Processing Options'
      end
      object Label36: TLabel
        Left = 24
        Top = 211
        Width = 217
        Height = 13
        Caption = 'Output Solution Status / Output Debug Trace'
      end
      object Label21: TLabel
        Left = 24
        Top = 187
        Width = 185
        Height = 13
        Caption = 'NMEA Interval (s) RMC/GGA, GSA/GSV'
        Enabled = False
      end
      object Label31: TLabel
        Left = 24
        Top = 163
        Width = 114
        Height = 13
        Caption = 'Solution for Static Mode'
      end
      object SolFormat: TComboBox
        Left = 248
        Top = 4
        Width = 152
        Height = 21
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 0
        Text = 'Lat/Lon/Height'
        OnChange = SolFormatChange
        Items.Strings = (
          'Lat/Lon/Height'
          'X/Y/Z-ECEF'
          'E/N/U-Baseline'
          'NMEA0183')
      end
      object TimeFormat: TComboBox
        Left = 248
        Top = 50
        Width = 118
        Height = 21
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 3
        Text = 'ww ssss GPST'
        Items.Strings = (
          'ww ssss GPST'
          'hh:mm:ss GPST'
          'hh:mm:ss UTC'
          'hh:mm:ss JST')
      end
      object LatLonFormat: TComboBox
        Left = 248
        Top = 72
        Width = 118
        Height = 21
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 4
        Text = 'ddd.dddddd'
        Items.Strings = (
          'ddd.dddddd'
          'ddd mm ss.ss')
      end
      object FieldSep: TEdit
        Left = 368
        Top = 72
        Width = 32
        Height = 21
        TabOrder = 5
      end
      object OutputDatum: TComboBox
        Left = 248
        Top = 116
        Width = 75
        Height = 21
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 6
        Text = 'WGS84'
        Items.Strings = (
          'WGS84')
      end
      object OutputHeight: TComboBox
        Left = 325
        Top = 116
        Width = 75
        Height = 21
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 7
        Text = 'Ellipsoidal'
        OnClick = OutputHeightClick
        Items.Strings = (
          'Ellipsoidal'
          'Geodetic')
      end
      object OutputGeoid: TComboBox
        Left = 248
        Top = 138
        Width = 152
        Height = 21
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 8
        Text = 'Internal'
        Items.Strings = (
          'Internal'
          'EGM96-BE (15")'
          'EGM2008-SE (2.5")'
          'EGM2008-SE (1.0")'
          'GSI2000 (1x1.5")')
      end
      object OutputHead: TComboBox
        Left = 248
        Top = 28
        Width = 75
        Height = 21
        Style = csDropDownList
        ItemIndex = 1
        TabOrder = 1
        Text = 'ON'
        Items.Strings = (
          'OFF'
          'ON')
      end
      object OutputOpt: TComboBox
        Left = 325
        Top = 28
        Width = 75
        Height = 21
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 2
        Text = 'OFF'
        Items.Strings = (
          'OFF'
          'ON')
      end
      object DebugTrace: TComboBox
        Left = 325
        Top = 208
        Width = 75
        Height = 21
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 9
        Text = 'OFF'
        Items.Strings = (
          'OFF'
          'Level1'
          'Level2'
          'Level3'
          'Level4'
          'Level5')
      end
      object DebugStatus: TComboBox
        Left = 248
        Top = 208
        Width = 75
        Height = 21
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 10
        Text = 'OFF'
        Items.Strings = (
          'OFF'
          'State'
          'Residuals')
      end
      object TimeDecimal: TEdit
        Left = 368
        Top = 50
        Width = 32
        Height = 21
        TabOrder = 11
        Text = '3'
      end
      object NmeaIntv1: TEdit
        Left = 248
        Top = 184
        Width = 75
        Height = 21
        Enabled = False
        TabOrder = 12
        Text = '0'
      end
      object NmeaIntv2: TEdit
        Left = 325
        Top = 184
        Width = 75
        Height = 21
        Enabled = False
        TabOrder = 13
        Text = '0'
      end
      object SolStatic: TComboBox
        Left = 248
        Top = 160
        Width = 152
        Height = 21
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 14
        Text = 'All'
        Items.Strings = (
          'All'
          'Single')
      end
      object OutputSingle: TComboBox
        Left = 248
        Top = 94
        Width = 75
        Height = 21
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 15
        Text = 'OFF'
        Items.Strings = (
          'OFF'
          'ON')
      end
      object MaxSolStd: TEdit
        Left = 325
        Top = 94
        Width = 75
        Height = 21
        TabOrder = 16
        Text = '0'
      end
    end
    object TabSheet4: TTabSheet
      Caption = 'S&tatistics'
      ImageIndex = 3
      object Label29: TLabel
        Left = 34
        Top = 213
        Width = 132
        Height = 13
        Caption = 'Satellite Clock Stability (s/s)'
      end
      object GroupBox3: TGroupBox
        Left = 2
        Top = 0
        Width = 397
        Height = 105
        Caption = 'Measurement Errors (1-sigma)'
        TabOrder = 0
        object Label6: TLabel
          Left = 35
          Top = 16
          Width = 179
          Height = 13
          Caption = 'Code/Carrier-Phase Error Ratio L1/L2'
        end
        object Label7: TLabel
          Left = 35
          Top = 38
          Width = 160
          Height = 13
          Caption = 'Carrier-Phase Error a+b/sinEl (m)'
        end
        object Label16: TLabel
          Left = 34
          Top = 60
          Width = 184
          Height = 13
          Caption = 'Carrier-Phase Error/Baseline (m/10km)'
        end
        object Label64: TLabel
          Left = 34
          Top = 82
          Width = 114
          Height = 13
          Caption = 'Doppler Frequency (Hz)'
        end
        object MeasErrR1: TEdit
          Left = 246
          Top = 14
          Width = 73
          Height = 21
          TabOrder = 0
          Text = '100.0'
        end
        object MeasErr2: TEdit
          Left = 246
          Top = 36
          Width = 73
          Height = 21
          TabOrder = 1
          Text = '0.003'
        end
        object MeasErr3: TEdit
          Left = 321
          Top = 36
          Width = 73
          Height = 21
          TabOrder = 2
          Text = '0.003'
        end
        object MeasErr4: TEdit
          Left = 246
          Top = 58
          Width = 148
          Height = 21
          TabOrder = 3
          Text = '0.000'
        end
        object MeasErr5: TEdit
          Left = 246
          Top = 80
          Width = 148
          Height = 21
          TabOrder = 4
          Text = '0.100'
        end
        object MeasErrR2: TEdit
          Left = 321
          Top = 14
          Width = 73
          Height = 21
          TabOrder = 5
          Text = '100.0'
        end
      end
      object GroupBox4: TGroupBox
        Left = 2
        Top = 104
        Width = 397
        Height = 105
        Caption = 'Process Noises (1-sigma/sqrt(s))'
        TabOrder = 1
        object Label26: TLabel
          Left = 32
          Top = 38
          Width = 123
          Height = 13
          Caption = 'Carrier-Phase Bias (cycle)'
        end
        object Label27: TLabel
          Left = 32
          Top = 60
          Width = 172
          Height = 13
          Caption = 'Vertical Ionospheric Delay (m/10km)'
        end
        object Label28: TLabel
          Left = 32
          Top = 84
          Width = 144
          Height = 13
          Caption = 'Zenith Tropospheric Delay (m)'
        end
        object Label10: TLabel
          Left = 32
          Top = 16
          Width = 170
          Height = 13
          Caption = 'Receiver Accel Horiz/Vertical (m/s2)'
        end
        object PrNoise1: TEdit
          Left = 246
          Top = 36
          Width = 148
          Height = 21
          TabOrder = 2
          Text = '1.0E-04'
        end
        object PrNoise2: TEdit
          Left = 246
          Top = 58
          Width = 148
          Height = 21
          TabOrder = 3
          Text = '1.0E-03'
        end
        object PrNoise3: TEdit
          Left = 246
          Top = 80
          Width = 148
          Height = 21
          TabOrder = 4
          Text = '1.0E-04'
        end
        object PrNoise4: TEdit
          Left = 246
          Top = 14
          Width = 73
          Height = 21
          TabOrder = 0
          Text = '1.0E-04'
        end
        object PrNoise5: TEdit
          Left = 321
          Top = 14
          Width = 73
          Height = 21
          TabOrder = 1
          Text = '1.0E-04'
        end
      end
      object SatClkStab: TEdit
        Left = 248
        Top = 211
        Width = 148
        Height = 21
        TabOrder = 2
        Text = '5.0E-12'
      end
    end
    object TabSheet5: TTabSheet
      Caption = '&Positions'
      ImageIndex = 4
      ExplicitHeight = 234
      object Label4: TLabel
        Left = 12
        Top = 12
        Width = 3
        Height = 13
      end
      object Label30: TLabel
        Left = 10
        Top = 196
        Width = 93
        Height = 13
        Caption = 'Station Position File'
      end
      object BtnStaPosView: TSpeedButton
        Left = 355
        Top = 210
        Width = 21
        Height = 21
        Flat = True
        Glyph.Data = {
          3E020000424D3E0200000000000036000000280000000D0000000D0000000100
          1800000000000802000000000000000000000000000000000000FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFF00FFFFFF00000000000000000000000000000000000000
          0000000000000000000000000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
          FFFFFF808080808080808080808080808080FFFFFFFFFFFFFFFFFF000000FFFF
          FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFF000000FFFFFF00FFFFFF000000FFFFFF808080808080808080FFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
          FFFFFF808080808080808080808080808080808080808080FFFFFF000000FFFF
          FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFF000000FFFFFF00FFFFFF00000000000000000000000000000000000000
          0000000000000000000000000000FFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF00}
        OnClick = BtnStaPosViewClick
      end
      object GroupRefAnt: TGroupBox
        Left = 1
        Top = 95
        Width = 397
        Height = 99
        Caption = 'Base Station'
        TabOrder = 1
        object LabelRefAntD: TLabel
          Left = 240
          Top = 60
          Width = 76
          Height = 13
          Caption = 'Delta-E/N/U (m)'
        end
        object RefAntE: TEdit
          Left = 238
          Top = 74
          Width = 51
          Height = 21
          TabOrder = 7
          Text = '0'
        end
        object RefAntN: TEdit
          Left = 290
          Top = 74
          Width = 51
          Height = 21
          TabOrder = 8
          Text = '0'
        end
        object RefAntU: TEdit
          Left = 342
          Top = 74
          Width = 51
          Height = 21
          TabOrder = 9
          Text = '0'
        end
        object RefPos1: TEdit
          Left = 6
          Top = 36
          Width = 126
          Height = 21
          TabOrder = 1
          Text = '0'
        end
        object RefPos2: TEdit
          Left = 133
          Top = 36
          Width = 129
          Height = 21
          TabOrder = 2
          Text = '0'
        end
        object RefPos3: TEdit
          Left = 263
          Top = 36
          Width = 129
          Height = 21
          TabOrder = 3
          Text = '0'
        end
        object BtnRefPos: TButton
          Left = 372
          Top = 14
          Width = 21
          Height = 21
          Caption = '...'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWindowText
          Font.Height = -9
          Font.Name = 'Tahoma'
          Font.Style = []
          ParentFont = False
          TabOrder = 4
          OnClick = BtnRefPosClick
        end
        object RefAntPcv: TCheckBox
          Left = 6
          Top = 58
          Width = 155
          Height = 17
          Caption = 'Antenna Type (*: Auto)'
          TabOrder = 5
          OnClick = RovAntPcvClick
        end
        object RefAnt: TComboBox
          Left = 6
          Top = 74
          Width = 230
          Height = 21
          DropDownCount = 16
          TabOrder = 6
          OnClick = RefAntClick
        end
        object RefPosType: TComboBox
          Left = 6
          Top = 14
          Width = 137
          Height = 21
          Style = csDropDownList
          ItemIndex = 0
          TabOrder = 0
          Text = 'Lat/Lon/Height (deg/m)'
          OnChange = RefPosTypeChange
          Items.Strings = (
            'Lat/Lon/Height (deg/m)'
            'Lat/Lon/Height (dms/m)'
            'X/Y/Z-ECEF (m)'
            'Average of Single Position'
            'Get from Position File'
            'RINEX Header Postion')
        end
      end
      object StaPosFile: TEdit
        Left = 2
        Top = 210
        Width = 352
        Height = 21
        TabOrder = 2
      end
      object BtnStaPosFile: TButton
        Left = 378
        Top = 209
        Width = 21
        Height = 21
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 3
        OnClick = BtnStaPosFileClick
      end
      object GroupRovAnt: TGroupBox
        Left = 2
        Top = 0
        Width = 397
        Height = 99
        Caption = 'Rover'
        TabOrder = 0
        object LabelRovAntD: TLabel
          Left = 239
          Top = 60
          Width = 76
          Height = 13
          Caption = 'Delta-E/N/U (m)'
        end
        object RovAntE: TEdit
          Left = 238
          Top = 74
          Width = 51
          Height = 21
          TabOrder = 7
          Text = '0'
        end
        object RovAntN: TEdit
          Left = 290
          Top = 74
          Width = 51
          Height = 21
          TabOrder = 8
          Text = '0'
        end
        object RovAntU: TEdit
          Left = 342
          Top = 74
          Width = 51
          Height = 21
          TabOrder = 9
          Text = '0'
        end
        object RovPos1: TEdit
          Left = 6
          Top = 36
          Width = 126
          Height = 21
          TabOrder = 1
          Text = '0'
        end
        object RovPos2: TEdit
          Left = 133
          Top = 36
          Width = 129
          Height = 21
          TabOrder = 2
          Text = '0'
        end
        object RovPos3: TEdit
          Left = 263
          Top = 36
          Width = 129
          Height = 21
          TabOrder = 3
          Text = '0'
        end
        object BtnRovPos: TButton
          Left = 372
          Top = 13
          Width = 21
          Height = 21
          Caption = '...'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWindowText
          Font.Height = -9
          Font.Name = 'Tahoma'
          Font.Style = []
          ParentFont = False
          TabOrder = 4
          OnClick = BtnRovPosClick
        end
        object RovAntPcv: TCheckBox
          Left = 6
          Top = 58
          Width = 161
          Height = 17
          Caption = 'Antenna Type (*: Auto)'
          TabOrder = 5
          OnClick = RovAntPcvClick
        end
        object RovAnt: TComboBox
          Left = 6
          Top = 74
          Width = 230
          Height = 21
          DropDownCount = 16
          TabOrder = 6
          OnClick = RovAntClick
        end
        object RovPosType: TComboBox
          Left = 6
          Top = 14
          Width = 137
          Height = 21
          Style = csDropDownList
          Enabled = False
          ItemIndex = 0
          TabOrder = 0
          Text = 'Lat/Lon/Height (deg/m)'
          OnChange = RovPosTypeChange
          Items.Strings = (
            'Lat/Lon/Height (deg/m)'
            'Lat/Lon/Height (dms/m)'
            'X/Y/Z-ECEF (m)'
            'Average of Single Pos'
            'Get from Position File'
            'RINEX Header Position')
        end
      end
    end
    object TabSheet7: TTabSheet
      Caption = '&Files'
      ImageIndex = 6
      object Label1: TLabel
        Left = 6
        Top = 199
        Width = 100
        Height = 13
        Caption = 'Ionosphere Data File'
      end
      object BtnAntPcvView: TSpeedButton
        Left = 381
        Top = -1
        Width = 21
        Height = 15
        Flat = True
        Glyph.Data = {
          3E020000424D3E0200000000000036000000280000000D0000000D0000000100
          1800000000000802000000000000000000000000000000000000FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFF00FFFFFF00000000000000000000000000000000000000
          0000000000000000000000000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
          FFFFFF808080808080808080808080808080FFFFFFFFFFFFFFFFFF000000FFFF
          FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFF000000FFFFFF00FFFFFF000000FFFFFF808080808080808080FFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
          FFFFFF808080808080808080808080808080808080808080FFFFFF000000FFFF
          FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFF000000FFFFFF00FFFFFF00000000000000000000000000000000000000
          0000000000000000000000000000FFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF00}
        OnClick = BtnAntPcvViewClick
      end
      object Label38: TLabel
        Left = 6
        Top = 2
        Width = 250
        Height = 13
        Caption = 'Satellite/Receiver Antenna PCV File ANTEX/NGS PCV'
      end
      object BtnSatPcvView: TSpeedButton
        Left = 355
        Top = -1
        Width = 21
        Height = 15
        Flat = True
        Glyph.Data = {
          3E020000424D3E0200000000000036000000280000000D0000000D0000000100
          1800000000000802000000000000000000000000000000000000FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFF00FFFFFF00000000000000000000000000000000000000
          0000000000000000000000000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
          FFFFFF808080808080808080808080808080FFFFFFFFFFFFFFFFFF000000FFFF
          FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFF000000FFFFFF00FFFFFF000000FFFFFF808080808080808080FFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
          FFFFFF808080808080808080808080808080808080808080FFFFFF000000FFFF
          FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFF000000FFFFFF00FFFFFF00000000000000000000000000000000000000
          0000000000000000000000000000FFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF00}
        OnClick = BtnSatPcvViewClick
      end
      object Label63: TLabel
        Left = 6
        Top = 59
        Width = 72
        Height = 13
        Caption = 'Geoid Data File'
      end
      object Label15: TLabel
        Left = 6
        Top = 94
        Width = 65
        Height = 13
        Caption = 'DCB Data File'
      end
      object BtnDCBView: TSpeedButton
        Left = 381
        Top = 93
        Width = 21
        Height = 15
        Flat = True
        Glyph.Data = {
          3E020000424D3E0200000000000036000000280000000D0000000D0000000100
          1800000000000802000000000000000000000000000000000000FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFF00FFFFFF00000000000000000000000000000000000000
          0000000000000000000000000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
          FFFFFF808080808080808080808080808080FFFFFFFFFFFFFFFFFF000000FFFF
          FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFF000000FFFFFF00FFFFFF000000FFFFFF808080808080808080FFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
          FFFFFF808080808080808080808080808080808080808080FFFFFF000000FFFF
          FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFF000000FFFFFF00FFFFFF00000000000000000000000000000000000000
          0000000000000000000000000000FFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF00}
        OnClick = BtnDCBViewClick
      end
      object Label34: TLabel
        Left = 6
        Top = 164
        Width = 60
        Height = 13
        Caption = 'OTL BLQ File'
      end
      object BtnBLQFileView: TSpeedButton
        Left = 381
        Top = 163
        Width = 21
        Height = 15
        Flat = True
        Glyph.Data = {
          3E020000424D3E0200000000000036000000280000000D0000000D0000000100
          1800000000000802000000000000000000000000000000000000FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFF00FFFFFF00000000000000000000000000000000000000
          0000000000000000000000000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
          FFFFFF808080808080808080808080808080FFFFFFFFFFFFFFFFFF000000FFFF
          FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFF000000FFFFFF00FFFFFF000000FFFFFF808080808080808080FFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
          FFFFFF808080808080808080808080808080808080808080FFFFFF000000FFFF
          FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFF000000FFFFFF00FFFFFF00000000000000000000000000000000000000
          0000000000000000000000000000FFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF00}
        OnClick = BtnBLQFileViewClick
      end
      object BtnEOPView: TSpeedButton
        Left = 381
        Top = 128
        Width = 21
        Height = 15
        Flat = True
        Glyph.Data = {
          3E020000424D3E0200000000000036000000280000000D0000000D0000000100
          1800000000000802000000000000000000000000000000000000FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFF00FFFFFF00000000000000000000000000000000000000
          0000000000000000000000000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
          FFFFFF808080808080808080808080808080FFFFFFFFFFFFFFFFFF000000FFFF
          FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFF000000FFFFFF00FFFFFF000000FFFFFF808080808080808080FFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
          FFFFFF808080808080808080808080808080808080808080FFFFFF000000FFFF
          FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFF000000FFFFFF00FFFFFF00000000000000000000000000000000000000
          0000000000000000000000000000FFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF00}
        OnClick = BtnEOPViewClick
      end
      object Label17: TLabel
        Left = 6
        Top = 129
        Width = 65
        Height = 13
        Caption = 'EOP Data File'
      end
      object AntPcvFile: TEdit
        Left = 2
        Top = 38
        Width = 378
        Height = 21
        TabOrder = 0
      end
      object BtnAntPcvFile: TButton
        Left = 381
        Top = 37
        Width = 21
        Height = 23
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 1
        OnClick = BtnAntPcvFileClick
      end
      object BtnIonoFile: TButton
        Left = 381
        Top = 212
        Width = 21
        Height = 23
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 5
        OnClick = BtnIonoFileClick
      end
      object IonoFile: TEdit
        Left = 2
        Top = 213
        Width = 378
        Height = 21
        TabOrder = 4
      end
      object SatPcvFile: TEdit
        Left = 2
        Top = 16
        Width = 378
        Height = 21
        TabOrder = 6
      end
      object BtnSatPcvFile: TButton
        Left = 381
        Top = 15
        Width = 21
        Height = 23
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 7
        OnClick = BtnSatPcvFileClick
      end
      object GeoidDataFile: TEdit
        Left = 2
        Top = 73
        Width = 378
        Height = 21
        TabOrder = 2
      end
      object BtnGeoidDataFile: TButton
        Left = 381
        Top = 72
        Width = 21
        Height = 23
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 3
        OnClick = BtnGeoidDataFileClick
      end
      object DCBFile: TEdit
        Left = 2
        Top = 108
        Width = 378
        Height = 21
        TabOrder = 8
      end
      object BtnDCBFile: TButton
        Left = 381
        Top = 107
        Width = 21
        Height = 23
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 9
        OnClick = BtnDCBFileClick
      end
      object BLQFile: TEdit
        Left = 2
        Top = 178
        Width = 378
        Height = 21
        TabOrder = 10
      end
      object BtnBLQFile: TButton
        Left = 381
        Top = 177
        Width = 21
        Height = 23
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 11
        OnClick = BtnBLQFileClick
      end
      object EOPFile: TEdit
        Left = 2
        Top = 143
        Width = 378
        Height = 21
        TabOrder = 12
      end
      object BtnEOPFile: TButton
        Left = 381
        Top = 142
        Width = 21
        Height = 23
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 13
        OnClick = BtnEOPFileClick
      end
    end
    object TabSheet6: TTabSheet
      Caption = '&Misc'
      ImageIndex = 6
      ExplicitHeight = 234
      object Label19: TLabel
        Left = 184
        Top = 136
        Width = 34
        Height = 13
        Caption = 'Rovers'
      end
      object Label39: TLabel
        Left = -96
        Top = 702
        Width = 37
        Height = 13
        Caption = 'Label39'
      end
      object Label40: TLabel
        Left = 38
        Top = 52
        Width = 150
        Height = 13
        Caption = 'SBAS Satellite Selection (0: All) '
      end
      object Label41: TLabel
        Left = 38
        Top = 8
        Width = 189
        Height = 13
        Caption = 'Time Interpolation of Base Station Data'
      end
      object Label42: TLabel
        Left = 38
        Top = 30
        Width = 121
        Height = 13
        Caption = 'DGPS/DGNSS Corrections'
        Enabled = False
      end
      object Label44: TLabel
        Left = 297
        Top = 137
        Width = 65
        Height = 13
        Caption = 'Base Stations'
      end
      object Label45: TLabel
        Left = 38
        Top = 137
        Width = 67
        Height = 13
        Caption = 'Station ID List'
      end
      object Label60: TLabel
        Left = 38
        Top = 74
        Width = 91
        Height = 13
        Caption = 'RINEX Opt (Rover)'
      end
      object Label12: TLabel
        Left = 38
        Top = 96
        Width = 85
        Height = 13
        Caption = 'RINEX Opt (Base)'
      end
      object Label23: TLabel
        Left = 38
        Top = 119
        Width = 58
        Height = 13
        Caption = 'PPP Options'
      end
      object RovList: TMemo
        Left = 143
        Top = 150
        Width = 127
        Height = 84
        Lines.Strings = (
          'rover')
        ScrollBars = ssVertical
        TabOrder = 0
      end
      object BaseList: TMemo
        Left = 273
        Top = 150
        Width = 127
        Height = 84
        Lines.Strings = (
          'base')
        ScrollBars = ssVertical
        TabOrder = 1
      end
      object IntpRefObs: TComboBox
        Left = 248
        Top = 4
        Width = 152
        Height = 21
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 2
        Text = 'OFF'
        OnChange = FreqChange
        Items.Strings = (
          'OFF'
          'ON')
      end
      object SbasSat: TEdit
        Left = 248
        Top = 48
        Width = 152
        Height = 21
        TabOrder = 3
        Text = '0'
      end
      object ComboBox1: TComboBox
        Left = 248
        Top = 26
        Width = 152
        Height = 21
        Style = csDropDownList
        Enabled = False
        ItemIndex = 0
        TabOrder = 4
        Text = 'SBAS'
        OnChange = FreqChange
        Items.Strings = (
          'SBAS'
          'RTCM')
      end
      object Panel1: TPanel
        Left = 38
        Top = 150
        Width = 103
        Height = 85
        BevelInner = bvRaised
        BevelOuter = bvLowered
        TabOrder = 5
        object BtnHelp: TSpeedButton
          Left = 4
          Top = 4
          Width = 15
          Height = 17
          Caption = '?'
          Flat = True
          OnClick = BtnHelpClick
        end
        object Label46: TLabel
          Left = 22
          Top = 4
          Width = 4
          Height = 13
          Caption = ':'
        end
        object Label62: TLabel
          Left = 32
          Top = 6
          Width = 61
          Height = 26
          Caption = 'Keywords in File Path'
          WordWrap = True
        end
        object Label43: TLabel
          Left = 6
          Top = 36
          Width = 20
          Height = 13
          Caption = '#..:'
        end
        object Label65: TLabel
          Left = 31
          Top = 37
          Width = 56
          Height = 26
          Caption = 'Comment in List'
          WordWrap = True
        end
      end
      object RnxOpts1: TEdit
        Left = 142
        Top = 70
        Width = 258
        Height = 21
        TabOrder = 6
      end
      object RnxOpts2: TEdit
        Left = 142
        Top = 92
        Width = 258
        Height = 21
        TabOrder = 7
      end
      object PPPOpts: TEdit
        Left = 142
        Top = 115
        Width = 258
        Height = 21
        TabOrder = 8
      end
    end
  end
  object BtnExtOpt: TButton
    Left = 1
    Top = 264
    Width = 69
    Height = 29
    Caption = 'Ext Opt...'
    Enabled = False
    TabOrder = 5
    Visible = False
    OnClick = BtnExtOptClick
  end
  object OpenDialog: TOpenDialog
    Filter = 
      'All (*.*)|*.*|PCV File (*.pcv,*.atx)|*.pcv;*.atx|Position File (' +
      '*.pos)|*.pos;*.pos|Options File (*.conf)|*.conf|DCB Data File (*' +
      '.dcb)|*.dcb|EOP Data File (*.eop,*erp)|*.eop;*.erp|OTL BLQ File ' +
      '(*.blq)|*.blq|Ionosphere Data File (*.*i,*stec)|*.*i;*.stec'
    Options = [ofHideReadOnly, ofNoChangeDir, ofEnableSizing]
    Title = 'Load File'
    Left = 81
    Top = 253
  end
  object SaveDialog: TSaveDialog
    Filter = 'All (*.*)|*.*|Options File (*.conf)|*.conf'
    Options = [ofHideReadOnly, ofNoChangeDir, ofEnableSizing]
    Title = 'Save File'
    Left = 51
    Top = 253
  end
end
