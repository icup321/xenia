/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2015 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_GPU_SHADER_TRANSLATOR_H_
#define XENIA_GPU_SHADER_TRANSLATOR_H_

#include <memory>
#include <string>
#include <vector>

#include "xenia/base/string_buffer.h"
#include "xenia/gpu/shader.h"
#include "xenia/gpu/ucode.h"
#include "xenia/gpu/xenos.h"

namespace xe {
namespace gpu {

class ShaderTranslator {
 public:
  virtual ~ShaderTranslator();

  // Gathers all vertex/texture bindings. Implicitly called in Translate.
  // DEPRECATED(benvanik): remove this when shader cache is removed.
  bool GatherAllBindingInformation(Shader* shader);

  bool Translate(Shader* shader, xenos::xe_gpu_program_cntl_t cntl);
  bool Translate(Shader* shader);

 protected:
  ShaderTranslator();

  // Resets translator state before beginning translation.
  virtual void Reset();

  // Register count.
  uint32_t register_count() const { return register_count_; }
  // True if the current shader is a vertex shader.
  bool is_vertex_shader() const { return shader_type_ == ShaderType::kVertex; }
  // True if the current shader is a pixel shader.
  bool is_pixel_shader() const { return shader_type_ == ShaderType::kPixel; }
  // A list of all vertex bindings, populated before translation occurs.
  const std::vector<Shader::VertexBinding>& vertex_bindings() const {
    return vertex_bindings_;
  }
  // A list of all texture bindings, populated before translation occurs.
  const std::vector<Shader::TextureBinding>& texture_bindings() const {
    return texture_bindings_;
  }

  // Current line number in the ucode disassembly.
  size_t ucode_disasm_line_number() const { return ucode_disasm_line_number_; }
  // Ucode disassembly buffer accumulated during translation.
  StringBuffer& ucode_disasm_buffer() { return ucode_disasm_buffer_; }
  // Emits a translation error that will be passed back in the result.
  virtual void EmitTranslationError(const char* message);
  // Emits a translation error indicating that the current translation is not
  // implemented or supported.
  virtual void EmitUnimplementedTranslationError();

  // Handles the start of translation.
  // At this point the vertex and texture bindings have been gathered.
  virtual void StartTranslation() {}

  // Handles the end of translation when all ucode has been processed.
  // Returns the translated shader binary.
  virtual std::vector<uint8_t> CompleteTranslation() {
    return std::vector<uint8_t>();
  }

  // Handles post-translation tasks when the shader has been fully translated.
  virtual void PostTranslation(Shader* shader) {}
  // Sets the host disassembly on a shader.
  void set_host_disassembly(Shader* shader, std::string value) {
    shader->host_disassembly_ = std::move(value);
  }

  // Pre-process a control-flow instruction before anything else.
  virtual void PreProcessControlFlowInstructions(
      std::vector<ucode::ControlFlowInstruction> instrs) {}

  // Handles translation for control flow label addresses.
  // This is triggered once for each label required (due to control flow
  // operations) before any of the instructions within the target exec.
  virtual void ProcessLabel(uint32_t cf_index) {}

  // Handles translation for control flow nop instructions.
  virtual void ProcessControlFlowNopInstruction(uint32_t cf_index) {}
  // Handles the start of a control flow instruction at the given address.
  virtual void ProcessControlFlowInstructionBegin(uint32_t cf_index) {}
  // Handles the end of a control flow instruction that began at the given
  // address.
  virtual void ProcessControlFlowInstructionEnd(uint32_t cf_index) {}
  // Handles translation for control flow exec instructions prior to their
  // contained ALU/fetch instructions.
  virtual void ProcessExecInstructionBegin(const ParsedExecInstruction& instr) {
  }
  // Handles translation for control flow exec instructions after their
  // contained ALU/fetch instructions.
  virtual void ProcessExecInstructionEnd(const ParsedExecInstruction& instr) {}
  // Handles translation for loop start instructions.
  virtual void ProcessLoopStartInstruction(
      const ParsedLoopStartInstruction& instr) {}
  // Handles translation for loop end instructions.
  virtual void ProcessLoopEndInstruction(
      const ParsedLoopEndInstruction& instr) {}
  // Handles translation for function call instructions.
  virtual void ProcessCallInstruction(const ParsedCallInstruction& instr) {}
  // Handles translation for function return instructions.
  virtual void ProcessReturnInstruction(const ParsedReturnInstruction& instr) {}
  // Handles translation for jump instructions.
  virtual void ProcessJumpInstruction(const ParsedJumpInstruction& instr) {}
  // Handles translation for alloc instructions.
  virtual void ProcessAllocInstruction(const ParsedAllocInstruction& instr) {}

  // Handles translation for vertex fetch instructions.
  virtual void ProcessVertexFetchInstruction(
      const ParsedVertexFetchInstruction& instr) {}
  // Handles translation for texture fetch instructions.
  virtual void ProcessTextureFetchInstruction(
      const ParsedTextureFetchInstruction& instr) {}
  // Handles translation for ALU instructions.
  virtual void ProcessAluInstruction(const ParsedAluInstruction& instr) {}

 private:
  struct AluOpcodeInfo {
    const char* name;
    size_t argument_count;
    int src_swizzle_component_count;
  };

  bool TranslateInternal(Shader* shader);

  void MarkUcodeInstruction(uint32_t dword_offset);
  void AppendUcodeDisasm(char c);
  void AppendUcodeDisasm(const char* value);
  void AppendUcodeDisasmFormat(const char* format, ...);

  bool TranslateBlocks();
  void GatherBindingInformation(const ucode::ControlFlowInstruction& cf);
  void GatherVertexBindingInformation(const ucode::VertexFetchInstruction& op);
  void GatherTextureBindingInformation(
      const ucode::TextureFetchInstruction& op);
  void TranslateControlFlowInstruction(const ucode::ControlFlowInstruction& cf);
  void TranslateControlFlowNop(const ucode::ControlFlowInstruction& cf);
  void TranslateControlFlowExec(const ucode::ControlFlowExecInstruction& cf);
  void TranslateControlFlowCondExec(
      const ucode::ControlFlowCondExecInstruction& cf);
  void TranslateControlFlowCondExecPred(
      const ucode::ControlFlowCondExecPredInstruction& cf);
  void TranslateControlFlowLoopStart(
      const ucode::ControlFlowLoopStartInstruction& cf);
  void TranslateControlFlowLoopEnd(
      const ucode::ControlFlowLoopEndInstruction& cf);
  void TranslateControlFlowCondCall(
      const ucode::ControlFlowCondCallInstruction& cf);
  void TranslateControlFlowReturn(
      const ucode::ControlFlowReturnInstruction& cf);
  void TranslateControlFlowCondJmp(
      const ucode::ControlFlowCondJmpInstruction& cf);
  void TranslateControlFlowAlloc(const ucode::ControlFlowAllocInstruction& cf);

  void TranslateExecInstructions(const ParsedExecInstruction& instr);

  void TranslateVertexFetchInstruction(const ucode::VertexFetchInstruction& op);
  void ParseVertexFetchInstruction(const ucode::VertexFetchInstruction& op,
                                   ParsedVertexFetchInstruction* out_instr);

  void TranslateTextureFetchInstruction(
      const ucode::TextureFetchInstruction& op);
  void ParseTextureFetchInstruction(const ucode::TextureFetchInstruction& op,
                                    ParsedTextureFetchInstruction* out_instr);

  void TranslateAluInstruction(const ucode::AluInstruction& op);
  void ParseAluVectorInstruction(const ucode::AluInstruction& op,
                                 const AluOpcodeInfo& opcode_info,
                                 ParsedAluInstruction& instr);
  void ParseAluScalarInstruction(const ucode::AluInstruction& op,
                                 const AluOpcodeInfo& opcode_info,
                                 ParsedAluInstruction& instr);

  // Input shader metadata and microcode.
  ShaderType shader_type_;
  const uint32_t* ucode_dwords_;
  size_t ucode_dword_count_;
  xenos::xe_gpu_program_cntl_t program_cntl_;
  uint32_t register_count_;

  // Accumulated translation errors.
  std::vector<Shader::Error> errors_;

  // Current control flow dword index.
  uint32_t cf_index_ = 0;

  // Microcode disassembly buffer, accumulated throughout the translation.
  StringBuffer ucode_disasm_buffer_;
  // Current line number in the disasm, which can be used for source annotation.
  size_t ucode_disasm_line_number_ = 0;
  // Last offset used when scanning for line numbers.
  size_t previous_ucode_disasm_scan_offset_ = 0;

  // Kept for supporting vfetch_mini.
  ucode::VertexFetchInstruction previous_vfetch_full_;

  // Detected binding information gathered before translation.
  int total_attrib_count_ = 0;
  std::vector<Shader::VertexBinding> vertex_bindings_;
  std::vector<Shader::TextureBinding> texture_bindings_;
  uint32_t unique_vertex_bindings_ = 0;
  uint32_t unique_texture_bindings_ = 0;

  Shader::ConstantRegisterMap constant_register_map_ = {0};
  bool writes_color_targets_[4] = {false, false, false, false};

  static const AluOpcodeInfo alu_vector_opcode_infos_[0x20];
  static const AluOpcodeInfo alu_scalar_opcode_infos_[0x40];
};

class UcodeShaderTranslator : public ShaderTranslator {
 public:
  UcodeShaderTranslator() = default;

 protected:
  std::vector<uint8_t> CompleteTranslation() override;
};

}  // namespace gpu
}  // namespace xe

#endif  // XENIA_GPU_SHADER_TRANSLATOR_H_
