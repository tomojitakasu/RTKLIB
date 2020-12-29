object TextViewer: TTextViewer
  Left = 0
  Top = 0
  Caption = 'TEXT VIEWER'
  ClientHeight = 415
  ClientWidth = 624
  Color = clWhite
  Constraints.MinHeight = 160
  Constraints.MinWidth = 320
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object Panel1: TPanel
    Left = 0
    Top = 0
    Width = 624
    Height = 29
    Align = alTop
    TabOrder = 0
    object Panel2: TPanel
      Left = 413
      Top = 1
      Width = 210
      Height = 27
      Align = alRight
      AutoSize = True
      BevelOuter = bvNone
      TabOrder = 0
      object BtnReload: TSpeedButton
        Left = 0
        Top = 0
        Width = 27
        Height = 27
        Hint = 'Reload'
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
        OnClick = BtnReloadClick
      end
      object BtnClose: TButton
        Left = 149
        Top = 0
        Width = 61
        Height = 27
        Align = alRight
        Caption = '&Close'
        TabOrder = 0
        OnClick = BtnCloseClick
      end
      object BtnRead: TButton
        Left = 27
        Top = 0
        Width = 61
        Height = 27
        Align = alRight
        Caption = '&Read...'
        TabOrder = 1
        OnClick = BtnReadClick
      end
      object BtnOpt: TButton
        Left = 88
        Top = 0
        Width = 61
        Height = 27
        Align = alRight
        Caption = '&Option...'
        TabOrder = 2
        OnClick = BtnOptClick
      end
    end
    object FindStr: TEdit
      Left = 2
      Top = 4
      Width = 119
      Height = 21
      TabOrder = 1
      OnKeyPress = FindStrKeyPress
    end
    object BtnFind: TButton
      Left = 122
      Top = 1
      Width = 46
      Height = 27
      Caption = 'Find'
      TabOrder = 2
      OnClick = BtnFindClick
    end
  end
  object Text: TRichEdit
    AlignWithMargins = True
    Left = 3
    Top = 32
    Width = 618
    Height = 380
    Align = alClient
    BevelInner = bvNone
    BevelOuter = bvNone
    BorderStyle = bsNone
    HideSelection = False
    Lines.Strings = (
      'No Text')
    PlainText = True
    ReadOnly = True
    ScrollBars = ssBoth
    TabOrder = 1
    WordWrap = False
    ExplicitLeft = 0
    ExplicitTop = 29
    ExplicitWidth = 624
    ExplicitHeight = 386
  end
  object OpenDialog: TOpenDialog
    Filter = 'Text File (*.txt)|*.txt|All (*.*)|*.*'
    Options = [ofHideReadOnly, ofNoChangeDir, ofEnableSizing]
    Left = 584
    Top = 368
  end
  object SaveDialog: TSaveDialog
    Filter = 'Text File (*.txt)|*.txt|All (*.*)|*.*'
    Options = [ofOverwritePrompt, ofHideReadOnly, ofNoChangeDir, ofEnableSizing]
    Left = 556
    Top = 368
  end
end
