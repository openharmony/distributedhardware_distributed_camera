/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * HarmonyCast Studio 设计令牌 — Broadcast Control Room 主题
 * 详见 docs/UX-DESIGN-SPEC.md
 */
export class DesignTokens {
  // Background
  public static readonly BG_PRIMARY: string = '#0B0F14';
  public static readonly BG_ELEVATED: string = '#151B24';
  public static readonly BG_GLASS: string = '#1A2230CC';
  public static readonly BG_INPUT: string = '#0F1419';

  // Accent
  public static readonly ACCENT_PRIMARY: string = '#00E5C7';
  public static readonly ACCENT_SECONDARY: string = '#3D8BFF';
  public static readonly ACCENT_WARN: string = '#FFB020';
  public static readonly ACCENT_DANGER: string = '#FF5A5A';
  public static readonly ACCENT_PURPLE: string = '#A78BFA';

  // Text
  public static readonly TEXT_PRIMARY: string = '#F2F5FA';
  public static readonly TEXT_SECONDARY: string = '#8B95A8';
  public static readonly TEXT_MUTED: string = '#5C6678';

  // Border
  public static readonly BORDER_SUBTLE: string = '#FFFFFF14';
  public static readonly BORDER_ACTIVE: string = '#00E5C780';

  // Radius
  public static readonly RADIUS_CARD: number = 16;
  public static readonly RADIUS_BUTTON: number = 12;
  public static readonly RADIUS_CHIP: number = 20;
  public static readonly RADIUS_PREVIEW: number = 20;

  // Spacing
  public static readonly SPACE_PAGE: number = 16;
  public static readonly SPACE_MODULE: number = 12;
  public static readonly SPACE_INNER: number = 8;

  // Animation
  public static readonly ANIM_FAST: number = 200;
  public static readonly ANIM_NORMAL: number = 320;
  /** 底部工具面板上拉 */
  public static readonly ANIM_SHEET_OPEN: number = 420;
  public static readonly ANIM_SHEET_CLOSE: number = 300;
  public static readonly ANIM_BACKDROP: number = 280;
}
