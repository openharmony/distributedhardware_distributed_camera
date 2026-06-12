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

import { common } from '@kit.AbilityKit';

/**
 * 全局 Context 持有者 — Service 层访问 Kit 时使用
 */
export class AppContext {
  private static uiAbilityContext: common.UIAbilityContext | null = null;
  private static foregroundListener: (() => void) | null = null;
  private static backgroundListener: (() => void) | null = null;

  static set(context: common.UIAbilityContext): void {
    AppContext.uiAbilityContext = context;
  }

  static get(): common.UIAbilityContext {
    if (AppContext.uiAbilityContext === null) {
      throw new Error('AppContext not initialized');
    }
    return AppContext.uiAbilityContext;
  }

  static tryGet(): common.UIAbilityContext | null {
    return AppContext.uiAbilityContext;
  }

  static setLifecycleListeners(onForeground: () => void, onBackground: () => void): void {
    AppContext.foregroundListener = onForeground;
    AppContext.backgroundListener = onBackground;
  }

  static notifyForeground(): void {
    AppContext.foregroundListener?.();
  }

  static notifyBackground(): void {
    AppContext.backgroundListener?.();
  }
}
