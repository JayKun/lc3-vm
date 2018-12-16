<h1>Virtual Machine</h1>
<h2>LC-3 Architecture (&quot;Hardware&quot;)</h2>
<h3>Memory</h3>
<pre><code class='language-c++' lang='c++'>#define MAXMEMORY UINT16_MAX
uint16_t memory[MAXMEMORY];
</code></pre>
<h3>16-bit Registers</h3>
<ul>
<li><strong>General Purpose Registers (R0 - R7)</strong></li>

</ul>
<ul>
<li><strong>Program Counter (PC) register</strong>: Unsigned integer that stores the memory address of the next instruction in memory</li>
<li><strong>Conditional flag register:</strong> Stores results about conditional tests</li>

</ul>
<pre><code class='language-c++' lang='c++'>enum
{
	R_R0 = 0,
    R_R1,
    R_R2,
    R_R3,
    R_R4,
    R_R5,
    R_R6,
    R_R7,
    R_PC,
    R_COND,
    R_COUNT
};
</code></pre>
<p>For easy access, we store the registers in an array</p>
<pre><code class='language-c++' lang='c++'>uint16_t reg[R_COUNT];
</code></pre>
<h2>Instruction Set</h2>
<p><strong>Opcode:</strong> Define fundamental tasks that a CPU is able to perform</p>
<pre><code class='language-c++' lang='c++'>enum
{
    OP_BR = 0, /* branch */
    OP_ADD,    /* add  */
    OP_LD,     /* load */
    OP_ST,     /* store */
    OP_JSR,    /* jump register */
    OP_AND,    /* bitwise and */
    OP_LDR,    /* load register */
    OP_STR,    /* store register */
    OP_RTI,    /* unused */
    OP_NOT,    /* bitwise not */
    OP_LDI,    /* load indirect */
    OP_STI,    /* store indirect */
    OP_JMP,    /* jump */
    OP_RES,    /* reserved (unused) */
    OP_LEA,    /* load effective address */
    OP_TRAP    /* execute trap */
};
</code></pre>
<h4>Conditional Flags</h4>
<p>The <strong>R_COND</strong> register stores conditional flags which provide information about the most recently executed calculation. This allows programs to check logical conditions. Each CPU has a variety of condition flags to signal various situations. The LC-3 uses only 3 condition flags which indicate the sign of the previous calculations.</p>
<pre><code class='language-c++' lang='c++'>enum
{
    FL_POS = 1 &lt;&lt; 0, /* P */
    FL_ZRO = 1 &lt;&lt; 1, /* Z */
    FL_NEG = 1 &lt;&lt; 2, /* N */
};
</code></pre>
<p>&nbsp;</p>

