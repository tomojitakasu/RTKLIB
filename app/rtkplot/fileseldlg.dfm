object FileSelDialog: TFileSelDialog
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  Caption = 'Solutions'
  ClientHeight = 507
  ClientWidth = 183
  Color = clWhite
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  OnResize = FormResize
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object Panel1: TPanel
    Left = 0
    Top = 0
    Width = 183
    Height = 19
    Align = alTop
    BevelOuter = bvNone
    TabOrder = 0
    object DriveSel: TDriveComboBox
      Left = 0
      Top = 0
      Width = 183
      Height = 19
      Align = alClient
      DirList = DirSel
      TabOrder = 0
      OnClick = DriveSelClick
    end
  end
  object Panel2: TPanel
    Left = 0
    Top = 19
    Width = 183
    Height = 18
    Align = alTop
    BevelInner = bvLowered
    BevelOuter = bvNone
    TabOrder = 1
    object Panel4: TPanel
      Left = 1
      Top = 1
      Width = 181
      Height = 16
      Align = alClient
      BevelInner = bvLowered
      BevelOuter = bvNone
      Color = clWhite
      TabOrder = 0
      OnClick = Panel4Click
      object DirLabel: TLabel
        Left = 4
        Top = 1
        Width = 120
        Height = 13
        Caption = 'C:\Users\ttaka\Desktop'
        OnClick = DirLabelClick
      end
      object BtnDirSel: TBitBtn
        Left = 162
        Top = 1
        Width = 18
        Height = 14
        Align = alRight
        Glyph.Data = {
          DE000000424DDE00000000000000360000002800000007000000070000000100
          180000000000A800000000000000000000000000000000000000FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFF000000FFFFFFFFFFFFFFFFFF000000FFFFFFFFFFFFFFFFFF00
          0000FFFFFFFFFFFF000000000000000000FFFFFFFFFFFF000000FFFFFF000000
          000000000000000000000000FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00
          0000}
        TabOrder = 0
        OnClick = BtnDirSelClick
      end
    end
  end
  object FileList: TFileListBox
    Left = 0
    Top = 37
    Width = 183
    Height = 449
    Align = alClient
    Constraints.MinHeight = 100
    ShowGlyphs = True
    TabOrder = 2
    OnClick = FileListClick
    OnMouseDown = FileListMouseDown
  end
  object Panel3: TPanel
    Left = 0
    Top = 486
    Width = 183
    Height = 21
    Align = alBottom
    BevelOuter = bvNone
    Caption = 'Panel3'
    TabOrder = 3
    object BtnUpdate: TSpeedButton
      Left = 164
      Top = 0
      Width = 19
      Height = 21
      Align = alRight
      Flat = True
      Glyph.Data = {
        DE000000424DDE0000000000000076000000280000000D0000000D0000000100
        0400000000006800000000000000000000001000000000000000000000000000
        8000008000000080800080000000800080008080000080808000C0C0C0000000
        FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00FFFFFFFFFFFF
        F000FFFFFFFFFFFFF000FFFF00007F0FF000FFF0FFFF000FF000FF7FFFF7000F
        F000FFFFFFFFF00FF000FF0FFFFFFF0FF000FF00FFFFFFFFF000FF0007FFFF7F
        F000FF000FFFF0FFF000FF0F70000FFFF000FFFFFFFFFFFFF000FFFFFFFFFFFF
        F000}
      OnClick = BtnUpdateClick
    end
    object Filter: TFilterComboBox
      Left = 0
      Top = 0
      Width = 164
      Height = 21
      Align = alClient
      FileList = FileList
      Filter = 'All (*.*)|*.*|Position File (*.pos, *.nmea)|*.pos;*.nmea'
      TabOrder = 0
      OnClick = FilterClick
    end
  end
  object Panel5: TPanel
    Left = 0
    Top = 36
    Width = 183
    Height = 327
    Margins.Left = 1
    Margins.Top = 1
    Margins.Right = 1
    Margins.Bottom = 1
    BevelInner = bvRaised
    BevelOuter = bvLowered
    Color = clWhite
    TabOrder = 4
    object DirSel: TDirectoryListBox
      Left = 2
      Top = 2
      Width = 179
      Height = 308
      Margins.Left = 1
      Margins.Top = 1
      Margins.Right = 1
      Margins.Bottom = 1
      Align = alClient
      BevelInner = bvNone
      BevelOuter = bvNone
      Constraints.MinHeight = 36
      DirLabel = DirLabel
      FileList = FileList
      IntegralHeight = True
      TabOrder = 0
      OnChange = DirSelChange
      OnDblClick = DirSelChange
    end
  end
end
