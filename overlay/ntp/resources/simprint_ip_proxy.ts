// Copyright 2025 Simprint
// Simprint IP detection proxy

import type {IpInfo, SimprintIpHandlerRemote} from './simprint_ip.mojom-webui.js';
import {SimprintIpHandler} from './simprint_ip.mojom-webui.js';

let handler: SimprintIpHandlerRemote|null = null;

export class SimprintIpProxy {
  static getHandler(): SimprintIpHandlerRemote {
    return handler || (handler = SimprintIpHandler.getRemote());
  }

  static setHandler(newHandler: SimprintIpHandlerRemote) {
    handler = newHandler;
  }

  static async getIpInfo(): Promise<IpInfo> {
    const result = await this.getHandler().getIpInfo();
    return result.info;
  }
}
