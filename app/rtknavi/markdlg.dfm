object MarkDialog: TMarkDialog
  Left = 0
  Top = 0
  BorderStyle = bsDialog
  Caption = 'Add Mark'
  ClientHeight = 206
  ClientWidth = 345
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
  object Label2: TLabel
    Left = 14
    Top = 55
    Width = 45
    Height = 13
    Caption = 'Comment'
  end
  object LabelPosMode: TLabel
    Left = 15
    Top = 141
    Width = 114
    Height = 13
    Caption = 'Switch Positioning Mode'
  end
  object BtnOk: TButton
    Left = 112
    Top = 171
    Width = 108
    Height = 33
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 1
    OnClick = BtnOkClick
  end
  object BtnCancel: TButton
    Left = 228
    Top = 171
    Width = 108
    Height = 32
    Caption = '&Cancel'
    Default = True
    ModalResult = 2
    TabOrder = 0
    OnClick = BtnCancelClick
  end
  object RadioStop: TRadioButton
    Left = 239
    Top = 139
    Width = 91
    Height = 17
    Caption = 'STOP (Static)'
    TabOrder = 5
  end
  object RadioGo: TRadioButton
    Left = 139
    Top = 139
    Width = 96
    Height = 17
    Caption = 'GO (Kinematic)'
    TabOrder = 4
  end
  object MarkerName: TComboBox
    Left = 12
    Top = 25
    Width = 321
    Height = 21
    TabOrder = 3
  end
  object ChkMarkerName: TCheckBox
    Left = 12
    Top = 7
    Width = 97
    Height = 17
    Caption = 'Marker Name'
    TabOrder = 2
    OnClick = ChkMarkerNameClick
  end
  object CommentText: TEdit
    Left = 12
    Top = 69
    Width = 321
    Height = 60
    AutoSelect = False
    AutoSize = False
    TabOrder = 6
  end
end
